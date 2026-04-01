"""End-to-end tests for zaberController::buildProfile().

Verifies that the driver sends the correct Zaber ASCII PVT command sequence
when an EPICS profile is built via caput to the standard motor profile PVs.
"""

import subprocess

import pytest

from tests.e2e.ca_helpers import get_int, get_str, put, wait_done
from tests.e2e.pvt_handler import PvtHandler

# PV prefixes matching motor.substitutions.xy-stage.zaber
P = "IOC:MyProfile:"
M1 = "M1"
M2 = "M2"

# Profile parameters for a simple 4-point, 2-axis move.
# Axis 1 ramps from 10 mm to 13 mm; axis 2 holds flat at 20 mm.
# Point 0 is the start position (consumed by runProfile's move-to-start,
# not submitted as a PVT point). Points 1–3 are submitted to the buffer,
# giving 3 wire-level "pvt 1 point abs" commands.
_NUM_POINTS = 4
_FIXED_TIME_S = 0.5
_AXIS1_POSITIONS = [10.0, 11.0, 12.0, 13.0]
_AXIS2_POSITIONS = [20.0, 20.0, 20.0, 20.0]


@pytest.mark.asyncio
async def test_build_profile_no_pulses(
    ioc_process: subprocess.Popen[bytes],
    pvt_handler: PvtHandler,
) -> None:
    """buildProfile() sends the correct PVT setup and point commands.

    Asserts:
    - The sequence is disabled and the buffer is erased before setup.
    - The correct setup-store command is sent for 2 axes.
    - Exactly 3 point commands are submitted (4 points minus the start position).
    - The sequence is disabled again after submission.
    - BuildStatus is success (1).
    """
    # Configure profile PVs.
    await put(f"{P}NumPoints", _NUM_POINTS)
    await put(f"{P}TimeMode", 0)          # fixed-time mode
    await put(f"{P}FixedTime", _FIXED_TIME_S)
    await put(f"{P}MoveMode", 0)          # absolute
    await put(f"{P}NumPulses", 0)
    await put(f"{P}{M1}UseAxis", 1)
    await put(f"{P}{M2}UseAxis", 1)
    await put(f"{P}{M1}Positions", _AXIS1_POSITIONS)
    await put(f"{P}{M2}Positions", _AXIS2_POSITIONS)

    # Flush any commands ZML sent during initialization (e.g. pvt 1 setup disable
    # on connection), so the queue only contains commands from this Build.
    pvt_handler.flush()

    # Trigger build and wait for completion.
    await put(f"{P}Build", 1)
    await wait_done(f"{P}Build")

    # Assert build succeeded before checking PVT commands.
    build_status = await get_int(f"{P}BuildStatus")
    assert build_status == 1, (
        f"BuildStatus={build_status}: {await get_str(f'{P}BuildMessage')}"
    )

    # Assert the exact PVT command sequence sent to the mock device.
    # ZML sends two back-to-back disables: our explicit sequence.disable() call,
    # then a second disable emitted internally by ZML's buffer.erase() call.
    await pvt_handler.expect_pvt("pvt 1 setup disable")
    await pvt_handler.expect_pvt("pvt 1 setup disable")
    await pvt_handler.expect_pvt("pvt buffer 1 erase")
    await pvt_handler.expect_pvt("pvt 1 setup store 1 2")

    # Points 1–3: axis 1 ramps from 10→11→12→13 mm; axis 2 holds at 20 mm.
    # Positions are in ZML microsteps; velocities in microsteps/s;
    # time in milliseconds (FixedTime=0.5 s → 500.0 ms).
    # Axis 2 velocity is 0 (flat) throughout; axis 1 velocity is 0 at the
    # final point (ramp-down to stop).
    await pvt_handler.expect_pvt("pvt 1 point abs p 88693 161260 v 31705 0 t 500.0")
    await pvt_handler.expect_pvt("pvt 1 point abs p 96756 161260 v 31705 0 t 500.0")
    await pvt_handler.expect_pvt("pvt 1 point abs p 104819 161260 v 0 0 t 500.0")

    await pvt_handler.expect_pvt("pvt 1 setup disable")

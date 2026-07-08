"""End-to-end tests for zaberController::buildProfile().

Verifies that the driver sends the correct Zaber ASCII PVT command sequence
when a coordinated motion profile is built via caput to the standard motor profile PVs.
"""

import pytest

from tests.e2e.profile_helpers import (
    M1,
    M2,
    PulseParams,
    assert_build_fails,
    assert_build_succeeds,
    configure_profile,
)
from tests.e2e.pvt_handler import PvtHandler

# Profile parameters for a simple 4-point, 2-axis move.
# Point 0 is the start position, (not submitted as a PVT point).
# Points 1-3 are submitted to the buffer,
_NUM_POINTS = 4
_FIXED_TIME_S = 0.5
_AXIS1_POSITIONS = [10.0, 11.0, 12.0, 13.0]
_AXIS2_POSITIONS = [20.0, 20.0, 20.0, 20.0]


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_build_profile_no_pulses(
    pvt_handler: PvtHandler,
) -> None:
    """zaberController::buildProfile sends the correct PVT setup and point commands."""
    await configure_profile(
        num_points=_NUM_POINTS,
        fixed_time=_FIXED_TIME_S,
        axis_positions={M1: _AXIS1_POSITIONS, M2: _AXIS2_POSITIONS},
    )

    # Flush any commands ZML sent during initialization, then build.
    pvt_handler.flush()
    await assert_build_succeeds()

    # Verify the PVT command sequence sent to the mock device.
    await pvt_handler.expect_pvt_skip("pvt 1 setup disable", "pvt buffer 1 erase")
    await pvt_handler.expect_pvt("pvt 1 setup store 1 2")

    # Verify points 1-3
    await pvt_handler.expect_pvt("pvt 1 point abs p 88693 161260 v 31705 0 t 500.0")
    await pvt_handler.expect_pvt("pvt 1 point abs p 96756 161260 v 31705 0 t 500.0")
    await pvt_handler.expect_pvt("pvt 1 point abs p 104819 161260 v 0 0 t 500.0")

    await pvt_handler.expect_pvt("pvt 1 setup disable")


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_build_profile_with_pulses(
    pvt_handler: PvtHandler,
) -> None:
    """zaberController::buildProfile interleaves digital-output IO actions with PVT points."""
    await configure_profile(
        num_points=_NUM_POINTS,
        fixed_time=_FIXED_TIME_S,
        pulses=PulseParams(num_pulses=2, start_pulses=2, end_pulses=3),
        axis_positions={M1: _AXIS1_POSITIONS, M2: _AXIS2_POSITIONS},
    )

    pvt_handler.flush()
    await assert_build_succeeds()

    # Skip the leading ``setup disable`` commands.
    await pvt_handler.expect_pvt_skip("pvt 1 setup disable", "pvt buffer 1 erase")
    await pvt_handler.expect_pvt("pvt 1 setup store 1 2")

    # Point idx 1 + pulse.
    await pvt_handler.expect_pvt("pvt 1 point abs p 88693 161260 v 31705 0 t 500.0")
    await pvt_handler.expect_pvt("pvt 1 io set do 1 1 schedule 10.0 0")
    # Point idx 2 + pulse.
    await pvt_handler.expect_pvt("pvt 1 point abs p 96756 161260 v 31705 0 t 500.0")
    await pvt_handler.expect_pvt("pvt 1 io set do 1 1 schedule 10.0 0")
    # Point idx 3 has no pulse.
    await pvt_handler.expect_pvt("pvt 1 point abs p 104819 161260 v 0 0 t 500.0")

    await pvt_handler.expect_pvt("pvt 1 setup disable")


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_build_profile_relative_mode_fails(
    pvt_handler: PvtHandler,
) -> None:
    """zaberController::buildProfile rejects relative move mode."""
    await configure_profile(
        num_points=_NUM_POINTS,
        fixed_time=_FIXED_TIME_S,
        move_mode=1,          # relative — unsupported
        axis_positions={M1: _AXIS1_POSITIONS},
    )

    pvt_handler.flush()
    await assert_build_fails("Relative move mode is not supported")

    # The build bailed out before touching the PVT sequence.
    assert pvt_handler.queue_is_empty(), (
        "Expected no PVT commands after a rejected relative-mode build"
    )


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
@pytest.mark.parametrize(
    "pulses",
    [
        pytest.param(PulseParams(2, 1, 2), id="start-pulse-at-profile-start"),
        pytest.param(PulseParams(2, 4, 5), id="end-pulse-past-last-point"),
        pytest.param(PulseParams(3, 2, 3), id="count-mismatch"),
    ],
)
async def test_build_profile_invalid_pulse_config_fails(
    pvt_handler: PvtHandler,
    pulses: PulseParams,
) -> None:
    """zaberController::buildProfile rejects pulse configs that violate constraints.

    Pulses must fire exactly at PVT points, so the driver enforces: 2 <= StartPulses,
    EndPulses <= NumPoints, and NumPulses == EndPulses - StartPulses + 1. Each parametrized
    case breaks one of these and must fail the build.
    """
    await configure_profile(
        num_points=_NUM_POINTS,
        fixed_time=_FIXED_TIME_S,
        pulses=pulses,
        axis_positions={M1: _AXIS1_POSITIONS},
    )

    pvt_handler.flush()
    await assert_build_fails("Invalid pulse configuration")

    assert pvt_handler.queue_is_empty(), (
        "Expected no PVT commands after a rejected pulse-config build"
    )

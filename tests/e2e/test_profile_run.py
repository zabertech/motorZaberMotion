"""End-to-end tests for zaberController abort and readback."""

import pytest

from tests.e2e.ca_helpers import get_float_array, get_int, get_message, put, wait_until_done
from tests.e2e.profile_helpers import (
    M1,
    M2,
    STATUS_SUCCESS,
    STEPS_PER_MM,
    P,
    PulseParams,
    assert_build_succeeds,
    configure_profile,
)
from tests.e2e.pvt_handler import PvtHandler

# A 4-point, 2-axis profile with pulses at points 2 and 3 (point indices 1, 2).
# Axis 1 ramps 10→13 mm; axis 2 holds at 20 mm.
_NUM_POINTS = 4
_FIXED_TIME_S = 0.5
_NUM_PULSES = 2
_START_PULSES = 2
_END_PULSES = 3
_AXIS1_POSITIONS = [10.0, 11.0, 12.0, 13.0]
_AXIS2_POSITIONS = [20.0, 20.0, 20.0, 20.0]

# The points that carry pulses (1-based StartPulses..EndPulses → indices 1, 2).
_AXIS1_PULSE_POSITIONS = [11.0, 12.0]
_AXIS2_PULSE_POSITIONS = [20.0, 20.0]

def _microstep_roundtrip(position_mm: float) -> float:
    """Quantize a position to the nearest microstep, exactly as readbackProfile does."""
    return round(position_mm * STEPS_PER_MM) / STEPS_PER_MM


async def _build_profile_with_pulses() -> None:
    """Build a standard 2-axis, pulsed profile and assert it succeeds."""
    await configure_profile(
        num_points=_NUM_POINTS,
        fixed_time=_FIXED_TIME_S,
        pulses=PulseParams(
            num_pulses=_NUM_PULSES, start_pulses=_START_PULSES, end_pulses=_END_PULSES
        ),
        axis_positions={M1: _AXIS1_POSITIONS, M2: _AXIS2_POSITIONS},
    )
    await assert_build_succeeds()


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_abort_profile_disables_sequence(
    pvt_handler: PvtHandler,
) -> None:
    """zaberController::abortProfile disables the PVT sequence on the device.

    A single ``pvt 1 setup disable`` is the entire abort handshake — it stops
    any in-progress replay and leaves the sequence idle.
    """
    await _build_profile_with_pulses()

    # Drop the build's commands so only the abort's command remains.
    pvt_handler.flush()

    await put(f"{P}Abort", 1)

    await pvt_handler.expect_pvt("pvt 1 setup disable")
    assert pvt_handler.queue_is_empty(), "Abort should emit exactly one disable command"


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_readback_profile_quantizes_pulse_positions() -> None:
    """zaberController::readbackProfile reports the pulse positions quantized to microsteps."""
    await _build_profile_with_pulses()

    await put(f"{P}Readback", 1)
    await wait_until_done(f"{P}Readback")

    readback_status = await get_int(f"{P}ReadbackStatus")
    assert readback_status == STATUS_SUCCESS, (
        f"ReadbackStatus={readback_status}: {await get_message(f'{P}ReadbackMessage')}"
    )

    # One readback per pulse, on each used axis.
    assert await get_int(f"{P}NumActualPulses") == _NUM_PULSES

    m1_readbacks = await get_float_array(f"{P}{M1}Readbacks", _NUM_PULSES)
    m1_errors = await get_float_array(f"{P}{M1}FollowingErrors", _NUM_PULSES)
    m2_readbacks = await get_float_array(f"{P}{M2}Readbacks", _NUM_PULSES)

    # Each readback must equal the requested position quantized to the nearest microstep.
    for got, req in zip(m1_readbacks, _AXIS1_PULSE_POSITIONS, strict=True):
        assert abs(got - _microstep_roundtrip(req)) < 1e-12, (
            f"M1 readback {got} != microstep round-trip of {req}"
        )
    for got, req in zip(m2_readbacks, _AXIS2_PULSE_POSITIONS, strict=True):
        assert abs(got - _microstep_roundtrip(req)) < 1e-12, (
            f"M2 readback {got} != microstep round-trip of {req}"
        )

    # FollowingError is the quantization error: (quantized position) - requested.
    for err, req in zip(m1_errors, _AXIS1_PULSE_POSITIONS, strict=True):
        assert abs(err - (_microstep_roundtrip(req) - req)) < 1e-12, (
            f"M1 error {err} != expected quantization error for {req}"
        )

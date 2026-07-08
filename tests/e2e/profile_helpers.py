"""Shared helpers for configuring and building motor profiles in e2e tests."""

from dataclasses import dataclass

from tests.e2e.ca_helpers import get_int, get_message, put, wait_until_done

# PV prefixes matching motor.substitutions.xy-stage.zaber.
P = "IOC:MyProfile:"
M1 = "M1"
M2 = "M2"

STEPS_PER_MM = 8062.992125984
"""Steps per mm for the mock's X-LHM linear peripheral at resolution 64."""

# Profile move status codes (from asynMotorController.h: ProfileStatus enum).
STATUS_SUCCESS = 1
STATUS_FAILURE = 2


@dataclass
class PulseParams:
    """Pulse-output configuration for a profile build."""

    num_pulses: int
    start_pulses: int
    end_pulses: int


async def configure_profile(
    *,
    num_points: int,
    axis_positions: dict[str, list[float]],
    move_mode: int = 0,
    time_mode: int = 0,
    fixed_time: float = 0.5,
    pulses: PulseParams | None = None,
) -> None:
    """Set the standard PVs for a motion profile.

    When ``pulses`` is None, no pulses are configured (NumPulses=0).
    """
    await put(f"{P}NumPoints", num_points)
    await put(f"{P}TimeMode", time_mode)
    await put(f"{P}FixedTime", fixed_time)
    await put(f"{P}MoveMode", move_mode)
    await put(f"{P}NumPulses", pulses.num_pulses if pulses is not None else 0)
    if pulses is not None:
        await put(f"{P}StartPulses", pulses.start_pulses)
        await put(f"{P}EndPulses", pulses.end_pulses)
    for motor, positions in axis_positions.items():
        await put(f"{P}{motor}UseAxis", 1)
        await put(f"{P}{motor}Positions", positions)


async def assert_build_succeeds() -> None:
    """Trigger Build, wait for completion, and assert it succeeded."""
    await put(f"{P}Build", 1)
    await wait_until_done(f"{P}Build")
    status = await get_int(f"{P}BuildStatus")
    assert status == STATUS_SUCCESS, (
        f"BuildStatus={status}: {await get_message(f'{P}BuildMessage')}"
    )


async def assert_build_fails(expected_message: str) -> None:
    """Trigger Build, wait, and assert it failed with expected_message in BuildMessage."""
    await put(f"{P}Build", 1)
    await wait_until_done(f"{P}Build")
    status = await get_int(f"{P}BuildStatus")
    assert status == STATUS_FAILURE, f"Expected build failure, got BuildStatus={status}"
    message = await get_message(f"{P}BuildMessage")
    assert expected_message in message, message

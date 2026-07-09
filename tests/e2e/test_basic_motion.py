"""End-to-end tests for zaberAxis basic motion commands (absolute/relative move, home)."""

import asyncio
import contextlib
from collections.abc import AsyncIterator, Callable

import pytest
from zaber_ascii_mock import MockDevice

from tests.e2e.ca_helpers import get_float, get_int, put
from tests.e2e.profile_helpers import STEPS_PER_MM

_AXIS = "XY_STAGE:axis1"
"""XY_STAGE:axis1 motor record from motor.substitutions.xy-stage.zaber (EGU = mm)."""

# MSTA status bits (from motor.h msta_field), set by zaberAxis::poll from ZML warning flags.
#
# See documentation on the MSTA field here for reference:
# https://bcda.xray.aps.anl.gov/synApps/motor/motorRecord.html#Fields_status
_MSTA_HOMED = 0x4000
_MSTA_HIGH_LIMIT = 0x4        # RA_PLUS_LS
_MSTA_FOLLOWING_ERROR = 0x40  # EA_SLIP_STALL
_MSTA_PROBLEM = 0x200         # RA_PROBLEM
_MSTA_LOW_LIMIT = 0x2000      # RA_MINUS_LS

@contextlib.asynccontextmanager
async def _completing_moves(mock: MockDevice) -> AsyncIterator[None]:
    """Complete in-progress mock moves so blocking motions finish."""
    stop = asyncio.Event()

    async def run() -> None:
        while not stop.is_set():
            for ax in mock._axes.values():  # noqa: SLF001
                if ax.is_busy:
                    ax.complete_move()
            await asyncio.sleep(0.02)

    task = asyncio.create_task(run())
    try:
        yield
    finally:
        stop.set()
        await task


def _microsteps(position_mm: float) -> int:
    """Return the native microstep target the driver sends for a position in mm."""
    return round(position_mm * STEPS_PER_MM)


async def _wait_until(check: Callable[[], bool], *, timeout: float = 10.0) -> None:
    """Poll a synchronous predicate until true, raising TimeoutError past the deadline."""
    loop = asyncio.get_running_loop()
    deadline = loop.time() + timeout
    while not check():
        if loop.time() > deadline:
            raise TimeoutError("condition not met in time")
        await asyncio.sleep(0.05)


async def _wait_msta(mask: int, *, present: bool = True, timeout: float = 10.0) -> None:
    """Poll MSTA until the masked bit is set or reset."""
    loop = asyncio.get_running_loop()
    deadline = loop.time() + timeout
    while True:
        msta = await get_int(f"{_AXIS}.MSTA")
        if bool(msta & mask) == present:
            return
        if loop.time() > deadline:
            raise TimeoutError(f"MSTA & {mask:#x} == {present} not reached (MSTA={msta:#x})")
        await asyncio.sleep(0.05)


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_absolute_move(mock_device: MockDevice) -> None:
    """Writing to VAL performs an absolute move."""
    async with _completing_moves(mock_device):
        await put(f"{_AXIS}.VAL", 5.0, wait=True, timeout=30)

    assert mock_device.axis(1).position == _microsteps(5.0)
    assert abs(await get_float(f"{_AXIS}.RBV") - 5.0) < 1e-9
    assert await get_int(f"{_AXIS}.DMOV") == 1


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_relative_move(mock_device: MockDevice) -> None:
    """Writing to RLV performs a relative move."""
    async with _completing_moves(mock_device):
        await put(f"{_AXIS}.VAL", 5.0, wait=True, timeout=30)   # start at 5 mm
        await put(f"{_AXIS}.RLV", 2.0, wait=True, timeout=30)   # end at 7 mm

    assert mock_device.axis(1).position == _microsteps(7.0)
    assert abs(await get_float(f"{_AXIS}.RBV") - 7.0) < 1e-9


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_home(mock_device: MockDevice) -> None:
    """Writing to HOMF homes the axis and sets the homed status bit."""
    mock_device.axis(1).position = _microsteps(5.0) # start away from home

    async with _completing_moves(mock_device):
        await put(f"{_AXIS}.HOMF", 1, wait=True, timeout=30)

    assert mock_device.axis(1).position == 0
    assert abs(await get_float(f"{_AXIS}.RBV") - 0.0) < 1e-9
    assert await get_int(f"{_AXIS}.MSTA") & _MSTA_HOMED


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_jog_starts_velocity_move(mock_device: MockDevice) -> None:
    """Writing to JOGF starts a velocity move and the axis reports busy (DMOV=0)."""
    await put(f"{_AXIS}.JOGF", 1)
    await _wait_until(lambda: mock_device.axis(1).is_busy)
    assert await get_int(f"{_AXIS}.DMOV") == 0
    await put(f"{_AXIS}.JOGF", 0)  # stop jogging


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_stop_commands_zero_velocity(
    mock_device: MockDevice, command_log: list[str]
) -> None:
    """Writing to STOP issues a zero-velocity move."""
    def _is_zero_velocity(command: str) -> bool:
        """Return True if command is a `move vel 0`."""
        parts = command.split()
        if parts[:2] != ["move", "vel"] or len(parts) < 3:
            return False
        try:
            return float(parts[2]) == 0.0
        except ValueError:
            return False

    await put(f"{_AXIS}.JOGF", 1)
    await _wait_until(lambda: mock_device.axis(1).is_busy)
    command_log.clear()
    await put(f"{_AXIS}.STOP", 1)
    await _wait_until(lambda: any(_is_zero_velocity(c) for c in command_log))


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_stall_flag_sets_following_error(mock_device: MockDevice) -> None:
    """A stall warning (FS) sets the MSTA following-error bit; poll clears it when it resolves."""
    mock_device.axis(1).warnings.add("FS")
    await _wait_msta(_MSTA_FOLLOWING_ERROR)
    mock_device.axis(1).warnings.discard("FS")
    await _wait_msta(_MSTA_FOLLOWING_ERROR, present=False)


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_limit_flag_sets_limit_bits(mock_device: MockDevice) -> None:
    """A limit-error warning (FE) sets both MSTA limit bits; poll clears them when it resolves."""
    mock_device.axis(1).warnings.add("FE")
    await _wait_msta(_MSTA_HIGH_LIMIT)
    await _wait_msta(_MSTA_LOW_LIMIT)
    mock_device.axis(1).warnings.discard("FE")
    await _wait_msta(_MSTA_HIGH_LIMIT, present=False)
    await _wait_msta(_MSTA_LOW_LIMIT, present=False)


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_fault_flag_sets_problem(mock_device: MockDevice) -> None:
    """A device fault flag sets the MSTA problem bit."""
    mock_device.axis(1).warnings.add("FF")
    await _wait_msta(_MSTA_PROBLEM)


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_unexpected_limit_warning_sets_problem(mock_device: MockDevice) -> None:
    """An unexpected-limit warning (WL) sets the MSTA problem bit."""
    mock_device.axis(1).warnings.add("WL")
    await _wait_msta(_MSTA_PROBLEM)


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_poll_does_not_clear_warnings(mock_device: MockDevice) -> None:
    """poll() reports warnings but does not clear them: a device flag persists across polls."""
    mock_device.axis(1).warnings.add("FE")
    await _wait_msta(_MSTA_PROBLEM)

    # Several idle-poll cycles (100 ms each) must leave the device flag in place.
    await asyncio.sleep(0.5)
    assert "FE" in mock_device.axis(1).warnings
    assert await get_int(f"{_AXIS}.MSTA") & _MSTA_PROBLEM


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_clear_warnings_record_clears_flags(mock_device: MockDevice) -> None:
    """Processing $(P)$(M):ClearWarnings clears the device flags and drops the problem bit."""
    mock_device.axis(1).warnings.add("FE")
    await _wait_msta(_MSTA_PROBLEM)

    await put(f"{_AXIS}:ClearWarnings", 1)

    await _wait_until(lambda: "FE" not in mock_device.axis(1).warnings)
    await _wait_msta(_MSTA_PROBLEM, present=False)


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_clear_warnings_repeats_without_reset(mock_device: MockDevice) -> None:
    """Clearing works repeatedly with no reset: each non-zero put clears again."""
    for _ in range(2):
        mock_device.axis(1).warnings.add("FE")
        await _wait_msta(_MSTA_PROBLEM)

        await put(f"{_AXIS}:ClearWarnings", 1)

        await _wait_until(lambda: "FE" not in mock_device.axis(1).warnings)
        await _wait_msta(_MSTA_PROBLEM, present=False)


@pytest.mark.asyncio
@pytest.mark.usefixtures("ioc_process")
async def test_clear_warnings_ignores_zero(mock_device: MockDevice) -> None:
    """A zero write is a no-op."""
    mock_device.axis(1).warnings.add("FE")
    await _wait_msta(_MSTA_PROBLEM)

    await put(f"{_AXIS}:ClearWarnings", 0)
    # A zero write doesn't clear.
    await asyncio.sleep(0.5)
    assert "FE" in mock_device.axis(1).warnings
    assert await get_int(f"{_AXIS}.MSTA") & _MSTA_PROBLEM

    # A non-zero write clears it.
    await put(f"{_AXIS}:ClearWarnings", 1)
    await _wait_until(lambda: "FE" not in mock_device.axis(1).warnings)
    await _wait_msta(_MSTA_PROBLEM, present=False)

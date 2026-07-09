"""E2e tests for linear EGU / unitsPerStep interpretation.

Runs against the XMCC_LINEAR controller from motor.substitutions.linear-test.zaber,
created with per-axis unitsPerStep "1.0 1.0 1.0 0.001":

  axis1  EGU=mm,  MRES=0.001, unitsPerStep=1.0    (baseline, 1 um step)
  axis3  EGU=um,  MRES=1.0,   unitsPerStep=1.0    (different EGU via MRES)
  axis4  EGU=mm,  MRES=1e-6,  unitsPerStep=0.001  (1 nm step, sub-micron resolution)

All three drive the same physical linear stage, so a given real-world position
maps to the same native microstep count regardless of EGU or step size.
"""

import asyncio
import contextlib
from collections.abc import AsyncIterator

import pytest
from zaber_ascii_mock import MockDevice

from tests.e2e.ca_helpers import get_float, put

pytestmark = pytest.mark.usefixtures("ioc_process")

STEPS_PER_MM = 8062.992125984

_MM_AXIS = "LINEAR_TEST:axis1"    # EGU = mm, unitsPerStep = 1.0
_UM_AXIS = "LINEAR_TEST:axis3"    # EGU = um, unitsPerStep = 1.0
_NM_AXIS = "LINEAR_TEST:axis4"    # EGU = mm, unitsPerStep = 0.001 (1 nm step)


def _microsteps(position_mm: float) -> int:
    """Native microstep target the driver sends for a physical position in mm."""
    return round(position_mm * STEPS_PER_MM)


def _roundtrip_mm(position_mm: float) -> float:
    """Physical mm after native-microstep quantization."""
    return _microsteps(position_mm) / STEPS_PER_MM


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


async def _move(pv: str, value: float, mock: MockDevice) -> None:
    async with _completing_moves(mock):
        await put(f"{pv}.VAL", value, wait=True, timeout=20)


async def test_millimetre_axis_baseline(mock_device: MockDevice) -> None:
    """Baseline mm axis: 5 mm -> the expected native microsteps."""
    await _move(_MM_AXIS, 5.0, mock_device)

    assert mock_device.axis(1).position == _microsteps(5.0)
    assert abs(await get_float(f"{_MM_AXIS}.RBV") - 5.0) < 1e-3


async def test_micron_egu_axis(mock_device: MockDevice) -> None:
    """EGU=um via MRES: 5000 um reaches the same physical position as 5 mm."""
    await _move(_UM_AXIS, 5000.0, mock_device)

    assert mock_device.axis(3).position == _microsteps(5.0)
    # RBV back in micrometres, within one native microstep.
    assert abs(await get_float(f"{_UM_AXIS}.RBV") - _roundtrip_mm(5.0) * 1000.0) < 0.2


async def test_nanometre_step_axis_reaches_same_position(mock_device: MockDevice) -> None:
    """unitsPerStep=0.001 (1 nm step) with MRES=1e-6 still maps 5 mm correctly."""
    await _move(_NM_AXIS, 5.0, mock_device)

    assert mock_device.axis(4).position == _microsteps(5.0)
    assert abs(await get_float(f"{_NM_AXIS}.RBV") - _roundtrip_mm(5.0)) < 1e-4


async def test_nanometre_step_resolves_sub_micron(mock_device: MockDevice) -> None:
    """The 1 nm step resolves a 0.5 um move that the 1 um-step axis would round away."""
    await _move(_NM_AXIS, 0.0005, mock_device)  # 0.5 um

    assert mock_device.axis(4).position == _microsteps(0.0005)
    assert mock_device.axis(4).position != 0

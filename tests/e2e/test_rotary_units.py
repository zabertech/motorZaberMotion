"""E2e tests for rotary units / MRES interpretation.

Runs against the XMCC_ROTARY controller (mock X-MCC4 with four rotary axes), created with
unitsPerStep "1e-6 1e-6 1e-6 1e-6" so every axis has a 1 micro-degree step. Each
axis displays a different angular EGU via its MRES:

  axis1  EGU=deg,    MRES=1e-6
  axis2  EGU=mdeg,   MRES=1e-3
  axis3  EGU=rad,    MRES=pi/180 * 1e-6
  axis4  EGU=arcsec, MRES=3.6e-3

All four drive the same physical rotary stage, so a given real-world angle maps to
the same native microstep count regardless of the EGU the record displays.
"""

import asyncio
import contextlib
import math
from collections.abc import AsyncIterator

import pytest
from zaber_ascii_mock import MockDevice

from tests.e2e.ca_helpers import get_float, put

pytestmark = pytest.mark.usefixtures("ioc_process", "reset_rotary_state")

# Native microsteps per degree for the mock's rotary peripheral (46657): a 0.5 deg
# move lands at 3200 microsteps, i.e. 6400 microsteps/deg.
MICROSTEPS_PER_DEG = 6400

_DEG_AXIS = "ROTARY_TEST:axis1"
_MDEG_AXIS = "ROTARY_TEST:axis2"
_RAD_AXIS = "ROTARY_TEST:axis3"
_ARCSEC_AXIS = "ROTARY_TEST:axis4"


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
    """Command an absolute move and block until it completes."""
    async with _completing_moves(mock):
        await put(f"{pv}.VAL", value, wait=True, timeout=20)

# The same physical angle (0.5 deg) expressed in each axis's EGU
_HALF_DEGREE_CASES = [
    (_DEG_AXIS, 1, 0.5),
    (_MDEG_AXIS, 2, 500.0),
    (_RAD_AXIS, 3, 0.5 * math.pi / 180.0),
    (_ARCSEC_AXIS, 4, 1800.0),
]

@pytest.mark.parametrize(("pv", "axis", "value"), _HALF_DEGREE_CASES)
async def test_rotary_egu_reaches_half_degree(
    rotary_mock_device: MockDevice, pv: str, axis: int, value: float
) -> None:
    """Each EGU commands and reads back correctly for the same 0.5 deg move."""
    await _move(pv, value, rotary_mock_device)

    assert rotary_mock_device.axis(axis).position == round(0.5 * MICROSTEPS_PER_DEG)
    assert abs(await get_float(f"{pv}.RBV") - value) < 1e-12


async def test_rotary_sub_degree_move_resolves(rotary_mock_device: MockDevice) -> None:
    """A sub-degree move resolves."""
    await _move(_DEG_AXIS, 0.005, rotary_mock_device)

    assert rotary_mock_device.axis(1).position == round(0.005 * MICROSTEPS_PER_DEG)
    assert abs(await get_float(f"{_DEG_AXIS}.RBV") - 0.005) < 1e-6


async def test_rotary_axes_independent_egus(rotary_mock_device: MockDevice) -> None:
    """Different-EGU axes on one controller move to distinct physical angles concurrently."""
    async with _completing_moves(rotary_mock_device):
        await put(f"{_DEG_AXIS}.VAL", 1.0, wait=True, timeout=20)  # 1 deg
        await put(f"{_ARCSEC_AXIS}.VAL", 1800.0, wait=True, timeout=20)  # 0.5 deg

    assert rotary_mock_device.axis(1).position == round(1.0 * MICROSTEPS_PER_DEG)
    assert rotary_mock_device.axis(4).position == round(0.5 * MICROSTEPS_PER_DEG)
    assert abs(await get_float(f"{_DEG_AXIS}.RBV") - 1.0) < 1e-6
    assert abs(await get_float(f"{_ARCSEC_AXIS}.RBV") - 1800.0) < 1e-3

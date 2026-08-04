"""E2e tests for rotary units / MRES interpretation."""

import math

import pytest
from zaber_ascii_mock import MockDevice

from tests.e2e.ca_helpers import get_float, put
from tests.e2e.mock_helpers import completing_moves, move

pytestmark = pytest.mark.usefixtures("ioc_process", "reset_rotary_state")

MICROSTEPS_PER_DEG = 6400
"""Native microsteps per degree for the mock's rotary peripheral (46657).

A 0.5 deg move lands at 3200 microsteps, i.e. 6400 microsteps/deg.
"""

# All rotary axes use stepScaleFactor = 1e-6 -> udeg precision
_DEG_AXIS = "ROTARY_TEST:axis1"     # EGU = deg,      MRES = 1e-6,     stepScaleFactor = 1e-6
_MDEG_AXIS = "ROTARY_TEST:axis2"    # EGU = millideg, MRES = 1e-3,     stepScaleFactor = 1e-6
_RAD_AXIS = "ROTARY_TEST:axis3"     # EGU = rad,      MRES = 1.745e-8, stepScaleFactor = 1e-6
_ARCSEC_AXIS = "ROTARY_TEST:axis4"  # EGU = arcsec,   MRES = 3.6e-3,   stepScaleFactor = 1e-6

_HALF_DEGREE_CASES = [
    (_DEG_AXIS, 1, 0.5),
    (_MDEG_AXIS, 2, 500.0),
    (_RAD_AXIS, 3, 0.5 * math.pi / 180.0),
    (_ARCSEC_AXIS, 4, 1800.0),
]
"""The same physical angle (0.5°) expressed in each axis's EGU."""

@pytest.mark.parametrize(("pv", "axis", "value"), _HALF_DEGREE_CASES)
async def test_rotary_egu_reaches_half_degree(
    rotary_mock_device: MockDevice, pv: str, axis: int, value: float
) -> None:
    """Each EGU commands and reads back correctly for the same 0.5 deg move."""
    await move(pv, value, rotary_mock_device)

    assert rotary_mock_device.axis(axis).position == round(0.5 * MICROSTEPS_PER_DEG)
    assert abs(await get_float(f"{pv}.RBV") - value) < 1e-14


async def test_rotary_sub_degree_move_resolves(rotary_mock_device: MockDevice) -> None:
    """A sub-degree move resolves."""
    await move(_DEG_AXIS, 0.005, rotary_mock_device)

    assert rotary_mock_device.axis(1).position == round(0.005 * MICROSTEPS_PER_DEG)
    assert abs(await get_float(f"{_DEG_AXIS}.RBV") - 0.005) < 1e-16


async def test_rotary_axes_independent_egus(rotary_mock_device: MockDevice) -> None:
    """Different-EGU axes on one controller move to distinct physical angles concurrently."""
    async with completing_moves(rotary_mock_device):
        await put(f"{_DEG_AXIS}.VAL", 1.0, wait=True, timeout=20)  # 1 deg
        await put(f"{_ARCSEC_AXIS}.VAL", 1800.0, wait=True, timeout=20)  # 0.5 deg

    assert rotary_mock_device.axis(1).position == round(1.0 * MICROSTEPS_PER_DEG)
    assert rotary_mock_device.axis(4).position == round(0.5 * MICROSTEPS_PER_DEG)
    assert abs(await get_float(f"{_DEG_AXIS}.RBV") - 1.0) < 1e-16
    assert abs(await get_float(f"{_ARCSEC_AXIS}.RBV") - 1800.0) < 1e-16

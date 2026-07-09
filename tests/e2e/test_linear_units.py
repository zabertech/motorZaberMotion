"""E2e tests for linear EGU / stepScaleFactor interpretation."""

import pytest
from zaber_ascii_mock import MockDevice

from tests.e2e.ca_helpers import get_float
from tests.e2e.mock_helpers import microsteps, move, roundtrip_mm

pytestmark = pytest.mark.usefixtures("ioc_process")

_MM_AXIS = "LINEAR_TEST:axis1"    # EGU = mm, stepScaleFactor = 1.0
_UM_AXIS = "LINEAR_TEST:axis3"    # EGU = um, stepScaleFactor = 1.0
_NM_AXIS = "LINEAR_TEST:axis4"    # EGU = mm, stepScaleFactor = 0.001 (1 nm step)


async def test_millimetre_axis_baseline(mock_device: MockDevice) -> None:
    """Baseline mm axis: 5 mm -> the expected native microsteps."""
    await move(_MM_AXIS, 5.0, mock_device)

    assert mock_device.axis(1).position == microsteps(5.0)
    assert abs(await get_float(f"{_MM_AXIS}.RBV") - 5.0) < 1e-12


async def test_micron_egu_axis(mock_device: MockDevice) -> None:
    """EGU=um via MRES: 5000 um reaches the same physical position as 5 mm."""
    await move(_UM_AXIS, 5000.0, mock_device)

    assert mock_device.axis(3).position == microsteps(5.0)
    # RBV back in micrometres, within one native microstep.
    assert abs(await get_float(f"{_UM_AXIS}.RBV") - roundtrip_mm(5.0) * 1000.0) < 0.2


async def test_nanometre_step_axis_reaches_same_position(mock_device: MockDevice) -> None:
    """stepScaleFactor=0.001 (1 nm step) with MRES=1e-6 still maps 5 mm correctly."""
    await move(_NM_AXIS, 5.0, mock_device)

    assert mock_device.axis(4).position == microsteps(5.0)
    assert abs(await get_float(f"{_NM_AXIS}.RBV") - roundtrip_mm(5.0)) < 1e-4


async def test_nanometre_step_resolves_sub_micron(mock_device: MockDevice) -> None:
    """The 1 nm step resolves a 0.5 um move that the 1 um-step axis would round away."""
    await move(_NM_AXIS, 0.0005, mock_device)  # 0.5 um

    assert mock_device.axis(4).position == microsteps(0.0005)
    assert mock_device.axis(4).position != 0

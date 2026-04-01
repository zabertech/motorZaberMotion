"""Pytest fixtures for motorZaberMotion end-to-end tests."""

import subprocess
from collections.abc import AsyncGenerator
from pathlib import Path

import pytest_asyncio
from zaber_ascii_mock import MockDevice
from zaber_ascii_mock.devices import xmcc4

from tests.e2e.ioc_helpers import render_st_cmd, start_ioc, stop_ioc, wait_for_ioc_ready
from tests.e2e.pvt_handler import PvtHandler


@pytest_asyncio.fixture
async def mock_device() -> AsyncGenerator[MockDevice, None]:
    """A running MockDevice presenting as an X-MCC4 with four linear axes.

    Both axes start at position 0. A fresh instance is created per test so
    each test starts with clean state.
    """
    async with MockDevice(device_number=1, info=xmcc4.FW7_38_FOUR_LINEAR) as mock:
        mock.axis(1).position = 0.0
        mock.axis(2).position = 0.0
        yield mock


@pytest_asyncio.fixture
async def pvt_handler(mock_device: MockDevice) -> PvtHandler:
    """A PvtHandler registered with mock_device before the IOC connects."""
    handler = PvtHandler(
        device_number=1,
        sequence_id=1,
        buffer_id=1,
        axes=mock_device._axes,  # noqa: SLF001
    )
    mock_device.register_command_handler("pvt", handler.handle)
    return handler


@pytest_asyncio.fixture
async def ioc_process(
    mock_device: MockDevice,
    pvt_handler: PvtHandler,  # ensure handler is registered before IOC connects
    tmp_path: Path,
) -> AsyncGenerator[subprocess.Popen[bytes], None]:
    """Start a real IOC pointed at mock_device, yield the process handle.

    Waits up to 30 s for the IOC to become ready before yielding.
    """
    st_cmd = render_st_cmd(tmp_path, mock_device.port)
    proc = start_ioc(st_cmd)
    try:
        await wait_for_ioc_ready()
        yield proc
    finally:
        stop_ioc(proc)

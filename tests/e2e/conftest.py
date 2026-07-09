"""Pytest fixtures for motorZaberMotion e2e tests.

Note that the mock device, PVT handler and IOC are all session-scoped
to save time so we don't have to start up an IOC for each test case.
The session-scoped event loop required for this is configured in pyproject.toml
(asyncio_default_fixture_loop_scope = session).
"""

import os
import subprocess
from collections.abc import AsyncGenerator

import pytest
import pytest_asyncio
from aioca import purge_channel_caches
from zaber_ascii_mock import MockDevice
from zaber_ascii_mock.devices import xmcc4
from zaber_ascii_mock.mock_device import WriteLineFn
from zaber_ascii_mock.protocol import Command
from zaber_ascii_mock.server import _default_port

from tests.e2e.ioc_helpers import render_st_cmd, start_ioc, stop_ioc, wait_for_ioc_ready
from tests.e2e.pvt_handler import PvtHandler

# Pin Channel Access (CA) to loopback for the whole test session. This is inherited
# by the IOC subprocess (server) as well as aioca (client), so the test IOC is neither
# discoverable on the LAN nor confused with any other IOC advertising the same PV names.
# It also silences the "Identical process variable names on multiple servers" warning that
# appears when one IOC is reachable via several interface addresses (on multi-homed hosts).
os.environ["EPICS_CA_AUTO_ADDR_LIST"] = "NO"
os.environ["EPICS_CA_ADDR_LIST"] = "localhost"
os.environ["EPICS_CAS_INTF_ADDR_LIST"] = "127.0.0.1"
os.environ["EPICS_CAS_BEACON_ADDR_LIST"] = "127.0.0.1"


@pytest_asyncio.fixture(scope="session")
async def mock_device() -> AsyncGenerator[MockDevice, None]:
    """Provide a MockDevice posing as an X-MCC4 with four linear axes."""
    async with MockDevice(device_number=1, info=xmcc4.FW7_38_FOUR_LINEAR) as mock:
        yield mock


@pytest_asyncio.fixture(scope="session")
async def pvt_handler(mock_device: MockDevice) -> PvtHandler:
    """Register a PvtHandler with mock_device before the IOC connects."""
    handler = PvtHandler(
        device_number=1,
        sequence_id=1,
        buffer_id=1,
        axes=mock_device._axes,  # noqa: SLF001
    )
    mock_device.register_command_handler("pvt", handler.handle)
    return handler


@pytest_asyncio.fixture(scope="session")
async def command_log(mock_device: MockDevice) -> list[str]:
    """Record every command the IOC sends, for wire-level assertions (passive tap)."""
    log: list[str] = []

    async def record(cmd: Command, _write_line: WriteLineFn) -> bool:
        log.append(cmd.data)
        return False  # never consume; other handlers and built-in dispatch still run

    mock_device.register_command_handler("", record)
    return log


@pytest_asyncio.fixture(scope="session")
async def rotary_mock_device() -> AsyncGenerator[MockDevice, None]:
    """Provide a MockDevice posing as an X-MCC4 with four rotary axes."""
    # Distinct port from the session mock_device (which uses the default) so both coexist.
    async with MockDevice(
        device_number=1, info=xmcc4.FW7_38_FOUR_ROTARY, port=_default_port() + 1
    ) as mock:
        yield mock


@pytest_asyncio.fixture(scope="session")
async def ioc_process(
    mock_device: MockDevice,
    rotary_mock_device: MockDevice,
    pvt_handler: PvtHandler,  # noqa: ARG001  # ensure handler is registered before IOC connects
    command_log: list[str],  # noqa: ARG001  # ensure command tap is registered before IOC connects
    tmp_path_factory: pytest.TempPathFactory,
) -> AsyncGenerator[subprocess.Popen[bytes], None]:
    """Start a real IOC pointed at two mock devices."""
    st_cmd = render_st_cmd(
        tmp_path_factory.mktemp("ioc"), mock_device.port, rotary_mock_device.port
    )
    proc = start_ioc(st_cmd)
    try:
        await wait_for_ioc_ready()
        yield proc
    finally:
        stop_ioc(proc)
        purge_channel_caches()


@pytest_asyncio.fixture
async def reset_rotary_state(rotary_mock_device: MockDevice) -> None:
    """Restore the rotary mock's axes to a clean state before each test that uses it."""
    for axis in rotary_mock_device._axes.values():  # noqa: SLF001
        axis.position = 0.0
        axis.is_busy = False
        axis.is_homed = False
        axis.warnings.clear()
        axis._target_position = None  # noqa: SLF001
        axis._move_done = None  # noqa: SLF001


@pytest_asyncio.fixture(autouse=True)
async def reset_state(
    mock_device: MockDevice,
    pvt_handler: PvtHandler,
    command_log: list[str],
) -> None:
    """Restore mock and handler to a clean state before each test.

    With session-scoped fixtures the mock device and PVT handler persist across
    tests, so any axis motion, queued commands or active-axis tracking left by
    a previous test must be cleared here to keep tests independent.
    """
    for axis in mock_device._axes.values():  # noqa: SLF001
        axis.position = 0.0
        axis.is_busy = False
        axis.is_homed = False
        axis.warnings.clear()
        axis._target_position = None  # noqa: SLF001
        axis._move_done = None  # noqa: SLF001
    pvt_handler.flush()
    pvt_handler._active_axes.clear()  # noqa: SLF001
    command_log.clear()

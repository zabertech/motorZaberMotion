"""zaber-ascii-mock — TCP mock server for Zaber ASCII protocol devices.

Public API::

    from zaber_ascii_mock import AsciiTcpServer, MockDevice
    from zaber_ascii_mock.identify import DeviceInfo, AxisInfo, Firmware, Protocol
    from zaber_ascii_mock.mock_axis import MockAxis
    from zaber_ascii_mock.protocol import Command, parse_command, format_response
    from zaber_ascii_mock.devices import xlhm, xmcc4

Two usage modes:

**Scripted** (low-level, for tests that control the full conversation)::

    async with AsciiTcpServer() as server:
        connection = Connection.open_tcp('127.0.0.1', server.port)
        await server.wait_for_connection()
        await run_identification_handshake(server, device_number=1, info=xlhm.FW7)
        line = await server.read_line()
        await server.write_line('@01 1 00 OK IDLE -- 0')

**Stateful dispatcher** (for drivers with a background polling thread)::

    async with MockDevice(device_number=1, info=xlhm.FW7) as mock:
        mock.axis(1).position = 0.0
        # Point your driver at tcp://127.0.0.1:{mock.port}
        # Drive operations via caput/ca_put/etc., then:
        await mock.axis(1).wait_until_idle()
"""

from .identify import AxisInfo, DeviceInfo, Firmware, Protocol, run_identification_handshake
from .mock_axis import MockAxis
from .mock_device import MockDevice
from .protocol import Command, format_response, parse_command
from .server import AsciiTcpServer, ConnectionClosedError

__all__ = [
    "AsciiTcpServer",
    "AxisInfo",
    "Command",
    "ConnectionClosedError",
    "DeviceInfo",
    "Firmware",
    "MockAxis",
    "MockDevice",
    "Protocol",
    "format_response",
    "parse_command",
    "run_identification_handshake",
]

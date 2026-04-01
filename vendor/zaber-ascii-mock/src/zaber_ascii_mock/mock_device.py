"""Stateful mock Zaber device.

MockDevice wraps AsciiTcpServer and handles:
  1. The ZML identification handshake automatically on each connection.
  2. A command dispatch loop that routes incoming Zaber ASCII commands
     to per-command handlers, returning appropriate responses based on
     the current MockAxis state.

This is the right abstraction for testing drivers that have a background
polling thread (like the EPICS motor driver), because the dispatch loop
runs concurrently and always has a response ready — no deadlock from
trying to pop() commands sequentially while polls arrive out of order.

Usage::

    async with MockDevice(device_number=1, info=devices.XLHM_FW7) as mock:
        mock.axis(1).position = 0.0
        # start your driver / IOC here, pointed at tcp://127.0.0.1:{mock.port}
        # drive moves, assert positions, etc.
        caput('LINEAR_STAGE:axis1.VAL', 50_000)  # µm → converted to microsteps by ZML
        await mock.axis(1).wait_until_idle()

Note on isBusy():
    ZML's isBusy() implementation sends a command to the device and checks
    the IDLE/BUSY field in the response. The exact command is not public.
    This dispatcher returns the axis's current busy state in the warning
    field of EVERY response for that axis, so regardless of which command
    ZML uses for the poll, the BUSY/IDLE status will be correct.

    Commands known to be used for status polling are handled explicitly.
    If ZML sends an unrecognised command, it is rejected with BADCOMMAND
    and a warning is logged — update _handle_command() accordingly.
"""

import asyncio
import contextlib
import logging
from collections.abc import Callable, Coroutine
from typing import Any, Self

from zaber_ascii_mock.identify import DeviceInfo, run_identification_handshake
from zaber_ascii_mock.mock_axis import MockAxis
from zaber_ascii_mock.protocol import Command, format_response, parse_command
from zaber_ascii_mock.server import AsciiTcpServer, ConnectionClosedError

type WriteLineFn = Callable[[str], Coroutine[Any, Any, None]]
"""Coroutine that writes one line to the connected client."""

type CommandHandler = Callable[[Command, WriteLineFn], Coroutine[Any, Any, bool]]
"""Custom command handler.

Receives the parsed command and a write_line coroutine.
Return True to consume the command, False to fall through to the built-in dispatch.
"""

logger = logging.getLogger(__name__)

# Minimum token count in a valid "set pos <value>" command.
_SET_POS_MIN_PARTS = 3


class MockDevice:
    """Stateful mock of a single Zaber device on the chain.

    Manages a TCP server, runs the identification handshake automatically
    on each connection, and dispatches incoming commands to per-command
    handlers that update and query MockAxis state.
    """

    def __init__(
        self,
        device_number: int,
        info: DeviceInfo,
        port: int | None = None,
    ) -> None:
        """Initialise the mock device."""
        self._device_number = device_number
        self._info = info
        self._server = AsciiTcpServer(port=port)
        self._axes: dict[int, MockAxis] = {
            i: MockAxis(axis_number=i)
            for i in range(1, len(info.axes) + 1)
        }
        self._accept_task: asyncio.Task[None] | None = None
        self._custom_handlers: list[tuple[str, CommandHandler]] = []

    @property
    def port(self) -> int:
        """TCP port the mock device is listening on."""
        return self._server.port

    def register_command_handler(self, prefix: str, handler: CommandHandler) -> None:
        """Register a handler for commands whose data starts with prefix.

        Handlers are tried in registration order before the built-in dispatch.
        The first handler that returns True consumes the command.
        """
        self._custom_handlers.append((prefix, handler))

    def axis(self, n: int) -> MockAxis:
        """Return the MockAxis for the given 1-based axis number."""
        return self._axes[n]

    async def __aenter__(self) -> Self:
        """Start the mock device server."""
        await self._server.start()
        self._accept_task = asyncio.create_task(self._accept_loop())
        return self

    async def __aexit__(self, *args: object) -> None:
        """Stop the mock device server."""
        if self._accept_task:
            self._accept_task.cancel()
            with contextlib.suppress(asyncio.CancelledError):
                await self._accept_task
        await self._server.stop()

    # ------------------------------------------------------------------
    # Internal — connection lifecycle
    # ------------------------------------------------------------------

    async def _accept_loop(self) -> None:
        """Accept connections, run identification, then spawn a dispatch task."""
        while True:
            try:
                await self._server.wait_for_connection()
                await run_identification_handshake(
                    self._server, self._device_number, self._info
                )
                asyncio.create_task(self._dispatch_loop())  # noqa: RUF006
            except asyncio.CancelledError:
                raise
            except Exception as exc:  # noqa: BLE001
                logger.warning("MockDevice: error during connection setup: %s", exc)

    async def _dispatch_loop(self) -> None:
        """Read commands and dispatch to handlers until the connection closes."""
        try:
            while True:
                line = await self._server.read_line()
                try:
                    cmd = parse_command(line)
                except ValueError as exc:
                    logger.warning("MockDevice: could not parse line %r: %s", line, exc)
                    continue
                try:
                    await self._handle_command(cmd)
                except Exception as exc:  # noqa: BLE001
                    logger.warning("MockDevice: error handling %r: %s", line, exc)
        except ConnectionClosedError:
            pass
        except asyncio.CancelledError:
            raise

    # ------------------------------------------------------------------
    # Internal — command dispatch
    # ------------------------------------------------------------------

    async def _handle_command(self, cmd: Command) -> None:  # noqa: C901, PLR0912, PLR0915
        d = self._device_number
        axis = self._axes.get(cmd.axis)
        busy = axis.is_busy if axis else False
        warning = "BUSY" if busy else "IDLE"

        async def reply(data: str = "0", *, status: str = "OK") -> None:
            await self._server.write_line(
                format_response(
                    d, cmd.axis, cmd.msg_id, status=status, warning=warning, data=data
                )
            )

        async def reject() -> None:
            await self._server.write_line(
                format_response(
                    d, cmd.axis, cmd.msg_id, status="RJ", warning="IDLE", data="BADCOMMAND"
                )
            )

        for prefix, handler in self._custom_handlers:
            if cmd.data.startswith(prefix):
                if await handler(cmd, self._server.write_line):
                    return

        data = cmd.data

        # Status/position poll — covers isBusy() and getPosition() in ZML
        if data in {"get pos", ""}:
            pos = int(axis.position) if axis else 0
            await reply(str(pos))

        # Warnings — covers getWarnings() in ZML
        elif data == "warnings":
            flags = " ".join(sorted(axis.warnings)) if (axis and axis.warnings) else "--"
            await reply(flags)

        elif data == "warnings clear":
            if axis:
                axis.warnings.clear()
            await reply()

        # Move commands
        elif data.startswith("move abs"):
            # Format: move abs <pos> [<vel> [<accel>]]
            parts = data.split()
            target = float(parts[2])
            if axis:
                axis.start_move(target)
            await reply()

        elif data.startswith("move rel"):
            parts = data.split()
            delta = float(parts[2])
            if axis:
                axis.start_move((axis.position if axis else 0.0) + delta)
            await reply()

        elif data.startswith("move vel"):
            # Velocity move: caller must call complete_move() to stop
            if axis:
                axis.start_velocity_move()
            await reply()

        elif data == "home":
            if axis:
                axis.start_move(0.0)
            await reply()

        # Set position (zaberAxis::setPosition → axis.getSettings().set("pos", ...))
        elif data.startswith("set pos"):
            parts = data.split()
            if axis and len(parts) >= _SET_POS_MIN_PARTS:
                axis.position = float(parts[2])
            await reply()

        # Settings get (used by poll for various things; reject with BADCOMMAND by default)
        elif data.startswith("get "):
            setting = data[4:]
            logger.debug("MockDevice: unhandled get %r — rejecting", setting)
            await reject()

        else:
            logger.warning(
                "MockDevice: unrecognised command %r on device %d axis %d — rejecting",
                data,
                d,
                cmd.axis,
            )
            await reject()

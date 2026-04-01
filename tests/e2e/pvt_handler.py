"""Custom command handler for Zaber ASCII PVT commands.

Register with MockDevice before the IOC connects::

    pvt = PvtHandler(device_number=1, sequence_id=1, buffer_id=1, axes=mock.axes)
    mock.register_command_handler("pvt", pvt.handle)

The handler auto-replies to every PVT command so the driver can make progress,
and enqueues each command for ordered assertion via expect_pvt() / drain_pvt().
"""

import asyncio
import logging
from collections.abc import Callable, Coroutine
from typing import Any

from zaber_ascii_mock.mock_axis import MockAxis
from zaber_ascii_mock.protocol import Command, format_response

logger = logging.getLogger(__name__)

type WriteLineFn = Callable[[str], Coroutine[Any, Any, None]]


class PvtHandler:
    """Handles all ``pvt …`` commands from a ZML-driven IOC.

    Thread-safety: designed for use in a single asyncio event loop.
    """

    def __init__(
        self,
        device_number: int,
        sequence_id: int,
        buffer_id: int,
        axes: dict[int, MockAxis],
    ) -> None:
        """Initialise the handler.

        Args:
            device_number: Zaber device number (1-indexed) that this handler serves.
            sequence_id:   PVT sequence ID used by the driver (typically 1).
            buffer_id:     PVT buffer ID used by the driver (typically 1).
            axes:          The MockDevice's axis dict, shared so complete_sequence()
                           can mark axes idle.
        """
        self._device_number = device_number
        self._seq = sequence_id
        self._buf = buffer_id
        self._axes = axes
        self._queue: asyncio.Queue[Command] = asyncio.Queue()
        self._active_axes: list[MockAxis] = []

    # ------------------------------------------------------------------
    # Handler — called by MockDevice dispatch loop
    # ------------------------------------------------------------------

    async def handle(self, cmd: Command, write_line: WriteLineFn) -> bool:
        """Respond to a pvt command and enqueue it for test assertions.

        Returns True always (consumes all pvt-prefixed commands).
        """
        d = self._device_number

        async def reply(warning: str = "IDLE") -> None:
            await write_line(
                format_response(d, cmd.axis, cmd.msg_id, warning=warning)
            )

        data = cmd.data

        if data == f"pvt {self._seq} setup disable":
            await reply()

        elif data == f"pvt buffer {self._buf} erase":
            await reply()

        elif data.startswith(f"pvt {self._seq} setup store"):
            await reply()

        elif data.startswith(f"pvt {self._seq} setup live"):
            # "pvt 1 setup live 1 2" — parse out the axis numbers
            parts = data.split()
            self._active_axes = [
                self._axes[int(n)] for n in parts[4:] if int(n) in self._axes
            ]
            await reply()

        elif data.startswith(f"pvt {self._seq} point"):
            await reply()

        elif data.startswith(f"pvt {self._seq} io"):
            await reply()

        elif data.startswith(f"pvt {self._seq} call"):
            # Mark participating axes busy so waitUntilIdle() polls see BUSY.
            for axis in self._active_axes:
                axis.start_velocity_move()
            await reply("BUSY")

        else:
            logger.warning("PvtHandler: unrecognised pvt command %r — rejecting", data)
            await write_line(
                format_response(
                    d, cmd.axis, cmd.msg_id, status="RJ", warning="IDLE", data="BADCOMMAND"
                )
            )

        await self._queue.put(cmd)
        return True

    # ------------------------------------------------------------------
    # Test control
    # ------------------------------------------------------------------

    def flush(self) -> None:
        """Discard all currently-queued PVT commands.

        Call this before triggering a driver action whose command sequence you
        want to assert, so that any commands sent during IOC initialization do
        not appear in the queue alongside the action's commands.
        """
        while not self._queue.empty():
            self._queue.get_nowait()

    def complete_sequence(self) -> None:
        """Signal that the PVT sequence has finished.

        Marks all axes that were set busy by the last ``call`` command as idle,
        unblocking ZML's waitUntilIdle() poll.
        """
        for axis in self._active_axes:
            axis.complete_move()

    # ------------------------------------------------------------------
    # Assertion helpers
    # ------------------------------------------------------------------

    async def expect_pvt(self, expected: str, *, timeout: float = 10.0) -> Command:
        """Pop the next PVT command and assert its data equals expected.

        Args:
            expected: The exact expected value of ``cmd.data``.
            timeout:  Seconds to wait for the command to arrive.

        Returns:
            The matched Command object.

        Raises:
            TimeoutError:   No command arrived within timeout.
            AssertionError: The command data did not match expected.
        """
        try:
            cmd = await asyncio.wait_for(self._queue.get(), timeout=timeout)
        except asyncio.TimeoutError as exc:
            raise TimeoutError(
                f"Timed out after {timeout}s waiting for PVT command {expected!r}"
            ) from exc
        assert cmd.data == expected, f"Expected {expected!r}, got {cmd.data!r}"
        return cmd

    async def drain_pvt(self, count: int, *, timeout: float = 10.0) -> list[Command]:
        """Pop count PVT commands without asserting on their content.

        Useful for consuming point submission commands whose exact microstep
        values are not yet known. Log the returned commands to discover the
        expected values for future expect_pvt() calls.

        Args:
            count:   Number of commands to consume.
            timeout: Seconds to wait for each command.

        Returns:
            List of consumed Command objects (in order).
        """
        commands: list[Command] = []
        for _ in range(count):
            try:
                cmd = await asyncio.wait_for(self._queue.get(), timeout=timeout)
            except asyncio.TimeoutError as exc:
                received = len(commands)
                raise TimeoutError(
                    f"Timed out after receiving {received}/{count} commands"
                ) from exc
            commands.append(cmd)
        return commands

"""Command handler for Zaber ASCII PVT commands."""

import asyncio
import logging
from collections.abc import Callable, Coroutine
from typing import Any

from zaber_ascii_mock.mock_axis import MockAxis
from zaber_ascii_mock.protocol import Command, format_response

logger = logging.getLogger(__name__)

type WriteLineFn = Callable[[str], Coroutine[Any, Any, None]]


class PvtHandler:
    """Handles all ``pvt ...`` commands from a Zaber IOC.

    Auto-replies to every PVT command so the driver can make progress, and enqueues
    each command for ordered assertion via expect_pvt().
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
            device_number: Zaber device number.
            sequence_id:   PVT sequence ID used by the driver.
            buffer_id:     PVT buffer ID used by the driver.
            axes:          The MockDevice's axis dictionary.
        """
        self._device_number = device_number
        self._seq = sequence_id
        self._buf = buffer_id
        self._axes = axes
        self._queue: asyncio.Queue[Command] = asyncio.Queue()
        self._active_axes: list[MockAxis] = []


    async def handle(self, cmd: Command, write_line: WriteLineFn) -> bool:
        """Respond to a pvt command and enqueue it for test assertions.

        Always returns True: this handler consumes every ``pvt`` command.
        """
        d = self._device_number

        async def reply(warning: str = "IDLE") -> None:
            await write_line(
                format_response(d, cmd.axis, cmd.msg_id, warning=warning)
            )

        data = cmd.data

        if data in {
            f"pvt {self._seq} setup disable",
            f"pvt buffer {self._buf} erase",
        } or data.startswith(f"pvt {self._seq} setup store"):
            await reply()

        elif data.startswith(f"pvt {self._seq} setup live"):
            # "pvt 1 setup live 1 2" — parse out the axis numbers
            parts = data.split()
            self._active_axes = [
                self._axes[int(n)] for n in parts[4:] if int(n) in self._axes
            ]
            await reply()

        elif data.startswith((f"pvt {self._seq} point", f"pvt {self._seq} io")):
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

    # Test controls

    def flush(self) -> None:
        """Discard all currently-queued PVT commands."""
        while not self._queue.empty():
            self._queue.get_nowait()

    def complete_sequence(self) -> None:
        """Signal that the PVT sequence has finished.

        Marks all axes that were set busy by the last ``call`` command as idle,
        unblocking ZML's waitUntilIdle() poll.
        """
        for axis in self._active_axes:
            axis.complete_move()

    # Assertion helpers

    async def expect_pvt(self, expected: str, *, timeout: float = 10.0) -> Command:
        """Pop the next PVT command and assert its data equals expected.

        Args:
            expected: The expected value of ``cmd.data``.
            timeout:  Seconds to wait for the command to arrive.

        Returns:
            The matched Command object.

        Raises:
            TimeoutError:   No command arrived within timeout.
            AssertionError: The command data did not match expected.
        """
        try:
            cmd = await asyncio.wait_for(self._queue.get(), timeout=timeout)
        except TimeoutError as exc:
            raise TimeoutError(
                f"Timed out after {timeout}s waiting for PVT command {expected!r}"
            ) from exc
        assert cmd.data == expected, f"Expected {expected!r}, got {cmd.data!r}"
        return cmd

    async def expect_pvt_skip(
        self, to_skip: str, expected: str, *, timeout: float = 10.0
    ) -> Command:
        """Skip commands and assert the first which doesn't match to_skip.

        Args:
            to_skip:  Command data to skip.
            expected: The exact expected value of the first non-skipped command.
            timeout:  Seconds to wait for each command.

        Returns:
            The matched Command object.
        """
        while True:
            try:
                cmd = await asyncio.wait_for(self._queue.get(), timeout=timeout)
            except TimeoutError as exc:
                raise TimeoutError(
                    f"Timed out after {timeout}s waiting for PVT command {expected!r}"
                ) from exc
            if cmd.data == to_skip:
                continue
            assert cmd.data == expected, (
                f"Expected {expected!r} (after skipping {to_skip!r}), got {cmd.data!r}"
            )
            return cmd

    def queue_is_empty(self) -> bool:
        """Return true if the queue is empty."""
        return self._queue.empty()

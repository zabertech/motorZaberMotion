"""Async TCP server that speaks line-oriented Zaber ASCII.

AsciiTcpServer is the low-level primitive — it provides a simple
read_line() / write_line() queue API over a TCP connection, equivalent
to the TestServer class in zaber-motion-lib's JS test suite.

Use it directly for scripted conversation tests where your test code
controls the exact exchange. Use MockDevice on top of it when you need
a stateful dispatcher (e.g. for drivers with a background polling thread).
"""

import asyncio
import contextlib
import os
import re
from typing import Self


class ConnectionClosedError(Exception):
    """Raised when a read or write is attempted on a closed connection."""


def _default_port() -> int:
    """Return a port that is stable per pytest-xdist worker, avoiding collisions."""
    worker = os.getenv("PYTEST_XDIST_WORKER", "")
    if worker.startswith("gw"):
        with contextlib.suppress(ValueError):
            return 22789 + int(worker[2:]) * 10
    return 22789


class AsciiTcpServer:
    """Async TCP server with a read_line() / write_line() interface.

    Accepts one connection at a time. When a new client connects, any
    existing connection is replaced (matching TestServer behaviour in
    the JS test suite).

    Usage as an async context manager::

        async with AsciiTcpServer() as server:
            # connect ZML here
            await server.wait_for_connection()
            line = await server.read_line()   # '/1 0 00 get deviceid'
            await server.write_line('@01 0 00 OK IDLE -- 50081')
    """

    host: str = "127.0.0.1"

    def __init__(self, port: int | None = None) -> None:
        """Initialise the server on the given port (default: worker-stable port)."""
        self.port = port if port is not None else _default_port()
        self._server: asyncio.Server | None = None
        self._writer: asyncio.StreamWriter | None = None
        self._read_task: asyncio.Task[None] | None = None
        self._line_queue: asyncio.Queue[str | None] = asyncio.Queue()
        self._connection_event: asyncio.Event = asyncio.Event()

    async def start(self) -> None:
        """Start listening for connections."""
        self._server = await asyncio.start_server(
            self._handle_connection, self.host, self.port
        )

    async def stop(self) -> None:
        """Stop the server and close any active connection."""
        self._close_connection()
        if self._server:
            self._server.close()
            await self._server.wait_closed()
            self._server = None

    async def __aenter__(self) -> Self:
        """Start the server."""
        await self.start()
        return self

    async def __aexit__(self, *args: object) -> None:
        """Stop the server."""
        await self.stop()

    async def wait_for_connection(self) -> None:
        """Block until a client connects."""
        await self._connection_event.wait()
        self._connection_event.clear()

    async def read_line(self) -> str:
        """Return the next line received from the client.

        Raises ConnectionClosedError if the connection was closed before a
        line arrived.
        """
        line = await self._line_queue.get()
        if line is None:
            raise ConnectionClosedError("Connection closed")  # noqa: TRY003
        return line

    async def write_line(self, line: str) -> None:
        """Send a line to the connected client (newline appended automatically)."""
        if self._writer is None:
            raise ConnectionClosedError("No active connection")  # noqa: TRY003
        self._writer.write(f"{line}\n".encode())
        await self._writer.drain()

    def close_connection(self) -> None:
        """Forcibly close the current client connection."""
        self._close_connection()

    # ------------------------------------------------------------------
    # Internal
    # ------------------------------------------------------------------

    def _close_connection(self) -> None:
        if self._read_task:
            self._read_task.cancel()
            self._read_task = None
        if self._writer:
            self._writer.close()
            self._writer = None
        # Unblock any pending read_line() calls
        self._line_queue.put_nowait(None)

    async def _handle_connection(
        self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter
    ) -> None:
        self._close_connection()
        self._writer = writer

        # Drain the sentinel we just put in and any leftover items
        while not self._line_queue.empty():
            self._line_queue.get_nowait()

        self._read_task = asyncio.create_task(self._read_loop(reader))
        self._connection_event.set()

    async def _read_loop(self, reader: asyncio.StreamReader) -> None:
        try:
            while True:
                raw = await reader.readline()
                if not raw:
                    break
                line = raw.decode().strip()
                # Strip optional checksum suffix (:XX)
                line = re.sub(r":.{2}$", "", line)
                if line:
                    await self._line_queue.put(line)
        except (asyncio.CancelledError, ConnectionResetError):
            pass
        finally:
            # Signal EOF to any waiting read_line() calls
            await self._line_queue.put(None)

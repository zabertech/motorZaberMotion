"""Typed wrappers around aioca channel access functions.

Keeps the Any boundary in one place so the rest of the test code is
fully type-annotated.
"""

import asyncio
from typing import Any

from aioca import caget, caput  # type: ignore[import-untyped]


async def get_int(pv: str, *, timeout: float = 5.0) -> int:
    """Return the current value of pv as an int."""
    result: Any = await caget(pv, timeout=timeout)
    return int(result)


async def get_float(pv: str, *, timeout: float = 5.0) -> float:
    """Return the current value of pv as a float."""
    result: Any = await caget(pv, timeout=timeout)
    return float(result)


async def get_str(pv: str, *, timeout: float = 5.0) -> str:
    """Return the current value of pv as a str."""
    result: Any = await caget(pv, timeout=timeout)
    return str(result)


async def put(pv: str, value: int | float | str | list[float], *, timeout: float = 5.0) -> None:
    """Write value to pv."""
    await caput(pv, value, timeout=timeout)


async def wait_done(pv: str, *, poll_interval: float = 0.2, timeout: float = 60.0) -> None:
    """Poll pv until its integer value is 0 (build/execute/readback done state).

    Raises:
        TimeoutError: pv did not reach 0 within timeout seconds.
    """
    deadline = asyncio.get_running_loop().time() + timeout
    while True:
        if await get_int(pv) == 0:
            return
        if asyncio.get_running_loop().time() > deadline:
            raise TimeoutError(f"{pv!r} did not reach 0 within {timeout}s")
        await asyncio.sleep(poll_interval)

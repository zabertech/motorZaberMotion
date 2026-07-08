"""Typed aioca channel access function wrappers."""

import asyncio

from aioca import caget, caput


async def get_int(pv: str, *, timeout: float = 5.0) -> int:
    """Return the current value of process variable as an int."""
    result = await caget(pv, timeout=timeout)
    return int(result)


async def get_float(pv: str, *, timeout: float = 5.0) -> float:
    """Return the current value of process variable as a float."""
    result = await caget(pv, timeout=timeout)
    return float(result)


async def get_str(pv: str, *, timeout: float = 5.0) -> str:
    """Return the current value of process variable as a str."""
    result = await caget(pv, timeout=timeout)
    return str(result)


async def get_message(pv: str, *, timeout: float = 5.0) -> str:
    """Decode a CHAR-waveform process variable (e.g. BuildMessage) into a Python string.

    The standard motor profile message PVs are ``waveform`` records with
    ``FTVL=CHAR``, so channel access returns an array of byte values. This
    reads that array and decodes it up to the first null terminator.
    """
    result = await caget(pv, timeout=timeout)
    raw = bytes(int(b) & 0xFF for b in result)
    return raw.split(b"\x00", 1)[0].decode("ascii", errors="replace")


async def get_float_array(pv: str, n: int, *, timeout: float = 5.0) -> list[float]:
    """Return the first n elements of a waveform PV as a list of floats."""
    result = await caget(pv, count=n, timeout=timeout)
    return [float(x) for x in result]


async def put(
    pv: str, value: float | str | list[float], *, wait: bool = False, timeout: float = 5.0
) -> None:
    """Write value to pv. With wait=True, block for put-completion (e.g. motion done)."""
    await caput(pv, value, wait=wait, timeout=timeout)


async def wait_until_done(pv: str, *, poll_interval: float = 0.2, timeout: float = 60.0) -> None:
    """Poll process variable until its integer value is 0 (build/execute/readback done state).

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

"""Helpers for driving the mock device during e2e tests."""

import asyncio
import contextlib
from collections.abc import AsyncIterator

from zaber_ascii_mock import MockDevice

from tests.e2e.ca_helpers import put

STEPS_PER_MM = 8062.992125984
"""Linear resolution of the mock's stage (native microsteps per mm)."""


def microsteps(position_mm: float) -> int:
    """Native microstep target the driver sends for a position in mm."""
    return round(position_mm * STEPS_PER_MM)


def roundtrip_mm(position_mm: float) -> float:
    """Return a position in mm after native-microstep quantization."""
    return microsteps(position_mm) / STEPS_PER_MM


@contextlib.asynccontextmanager
async def completing_moves(mock: MockDevice) -> AsyncIterator[None]:
    """Complete in-progress mock moves so blocking motions finish."""
    stop = asyncio.Event()

    async def run() -> None:
        while not stop.is_set():
            for ax in mock._axes.values():  # noqa: SLF001
                if ax.is_busy:
                    ax.complete_move()
            await asyncio.sleep(0.02)

    task = asyncio.create_task(run())
    try:
        yield
    finally:
        stop.set()
        await task


async def move(pv: str, value: float, mock: MockDevice) -> None:
    """Command an absolute move (write VAL) and block until it completes."""
    async with completing_moves(mock):
        await put(f"{pv}.VAL", value, wait=True, timeout=20)

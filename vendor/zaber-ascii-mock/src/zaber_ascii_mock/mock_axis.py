"""Stateful simulation of a single Zaber axis.

Position is stored in native units (microsteps), matching what goes
over the Zaber ASCII wire. ZML converts to/from physical units (µm,
degrees, etc.) using the device database.

Typical microstep conversions (for reference when writing test assertions):
  X-LHM (resolution 64, ~5µm/step): 1 mm ≈ varies by device db lookup
  Use the values from axis_identified.test.ts as ground truth, e.g.
  4.5 mm → 36283 microsteps for X-LHM FW7.
"""

import asyncio
from dataclasses import dataclass, field


@dataclass
class MockAxis:
    """Simulated state for one axis.

    Test code interacts with this object to set up initial state before
    an IOC starts, and to observe or trigger state changes during a test.
    """

    axis_number: int

    # Current position in native units (microsteps)
    position: float = 0.0

    is_busy: bool = False
    is_homed: bool = False

    # Active warning flags, e.g. {'WL', 'FS'}. Cleared by ZML's warnings.clearFlags().
    warnings: set[str] = field(default_factory=set[str])

    _target_position: float | None = field(default=None, repr=False)
    _move_done: asyncio.Event | None = field(default=None, repr=False)

    def start_move(self, target: float) -> None:
        """Mark the axis as busy and record the target position.

        The move does NOT complete until complete_move() is called.
        This gives tests explicit control over timing.
        """
        self.is_busy = True
        self._target_position = target
        self._move_done = asyncio.Event()

    def start_velocity_move(self) -> None:
        """Mark the axis as busy for a velocity move (no fixed target).

        The move does NOT complete until complete_move() is called.
        Position is not updated automatically — set axis.position directly
        before calling complete_move() if you need a specific final position.
        """
        self.is_busy = True
        self._target_position = None
        self._move_done = asyncio.Event()

    def complete_move(self) -> None:
        """Finish the current move: update position and mark as idle.

        If no move is in progress this is a no-op.
        """
        if not self.is_busy:
            return
        if self._target_position is not None:
            self.position = self._target_position
            self._target_position = None
        self.is_busy = False
        self.is_homed = True
        if self._move_done:
            self._move_done.set()
            self._move_done = None

    async def wait_until_idle(self) -> None:
        """Await completion of the current move (if one is in progress)."""
        if self._move_done:
            await self._move_done.wait()

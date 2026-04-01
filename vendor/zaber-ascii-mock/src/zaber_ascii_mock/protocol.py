"""Parsing and formatting for the Zaber ASCII protocol wire format.

Zaber ASCII command format (client → device):
    /<device> <axis> <id> <data>
    e.g. /1 1 00 move abs 36283

Zaber ASCII response format (device → client):
    @<device> <axis> <id> <status> <warning> <flags> <data>
    e.g. @01 1 00 OK IDLE -- 36283
"""

import re
from dataclasses import dataclass


@dataclass
class Command:
    """A parsed Zaber ASCII command received from ZML."""

    device: int
    axis: int
    msg_id: str
    data: str
    raw: str
    has_continuation: bool


def parse_command(line: str) -> Command:
    """Parse a Zaber ASCII command line sent by ZML to the device."""
    line = line.strip()
    # Strip optional checksum suffix (:XX)
    line = re.sub(r":.{2}$", "", line)

    match = re.match(r"^/(\d+) (\d+) (\w+) ?(.*)$", line)
    if not match:
        raise ValueError(f"Cannot parse command: {line!r}")  # noqa: TRY003

    device, axis, msg_id, data = match.groups()
    return Command(
        device=int(device),
        axis=int(axis),
        msg_id=msg_id,
        data=data,
        raw=line,
        has_continuation=data.endswith("\\"),
    )


def format_response(  # noqa: PLR0913
    device: int,
    axis: int,
    msg_id: str,
    *,
    status: str = "OK",
    warning: str = "IDLE",
    flags: str = "--",
    data: str = "0",
) -> str:
    """Format a Zaber ASCII response line to send back to ZML."""
    return f"@{device:02d} {axis} {msg_id} {status} {warning} {flags} {data}"

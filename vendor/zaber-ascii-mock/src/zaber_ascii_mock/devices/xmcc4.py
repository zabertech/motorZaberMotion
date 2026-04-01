"""DeviceInfo instances for the X-MCC4 4-axis controller (device ID 30342).

Ported from zaber-motion-lib/js/test-e2e/ascii/devices/xmcc4.ts.

Peripheral IDs:
  43211 — X-LHM linear stage
  46657 — rotary stage
  88800 — 3rd-party (unrecognised)
  0     — empty / no peripheral
"""

from zaber_ascii_mock.identify import AxisInfo, DeviceInfo, Firmware, Protocol

_LINE_CONT_PROTOCOL = Protocol(max_packets=5, word_len=50)
_GET_SYNC_PROTOCOL = Protocol(max_packets=5, word_len=50, max_sync_settings=12)

_FOUR_LINEAR = [AxisInfo(peripheral_id=43211, resolution=64)] * 4
_ONE_LINEAR = [
    AxisInfo(peripheral_id=70250, resolution=64),
    AxisInfo(peripheral_id=0, resolution=64),
    AxisInfo(peripheral_id=0, resolution=64),
    AxisInfo(peripheral_id=0, resolution=64),
]
_TWO_LINEAR_TWO_ROTARY = [
    AxisInfo(peripheral_id=43211, resolution=64),
    AxisInfo(peripheral_id=43211, resolution=64),
    AxisInfo(peripheral_id=46657, resolution=64),
    AxisInfo(peripheral_id=46657, resolution=64),
]


def _make(firmware: Firmware, axes: list[AxisInfo], protocol: Protocol | None) -> DeviceInfo:
    return DeviceInfo(
        device_id=30342,
        serial=1234,
        firmware=firmware,
        axes=axes,
        protocol=protocol,
    )


# FW7.21 — no extended protocol, four linear axes (most basic config)
FW7_21_FOUR_LINEAR = _make(
    Firmware("7.21", "9827"), _FOUR_LINEAR, None
)

# FW7.33 — line continuation protocol
FW7_33_FOUR_LINEAR = _make(
    Firmware("7.33", "14091"), _FOUR_LINEAR, _LINE_CONT_PROTOCOL
)

# FW7.38 — get-sync protocol (most capable)
FW7_38_FOUR_LINEAR = _make(
    Firmware("7.38", "15708"), _FOUR_LINEAR, _GET_SYNC_PROTOCOL
)

FW7_38_ONE_LINEAR = _make(
    Firmware("7.38", "15708"), _ONE_LINEAR, _GET_SYNC_PROTOCOL
)

FW7_38_TWO_LINEAR_TWO_ROTARY = _make(
    Firmware("7.38", "15708"), _TWO_LINEAR_TWO_ROTARY, _GET_SYNC_PROTOCOL
)

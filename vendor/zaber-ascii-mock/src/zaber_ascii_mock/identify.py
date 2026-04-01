"""Zaber device identification handshake.

This is a direct port of identifyDevice() from
zaber-motion-lib/js/test-e2e/ascii/devices/identify.ts.

ZML calls device.identify() once on startup, which triggers a fixed
sequence of get-response pairs. The server must reply in exactly this
order with data matching the DeviceInfo provided.

The device_id must correspond to a real Zaber device model known to the
ZML device database — ZML uses it to resolve unit conversions (e.g.
microsteps ↔ µm). Use the pre-built DeviceInfo instances in the
devices/ subpackage for known models.
"""

import base64
import json
from dataclasses import dataclass, field

from zaber_ascii_mock.protocol import Command, format_response, parse_command
from zaber_ascii_mock.server import AsciiTcpServer

# Firmware build number at which ZML begins requesting device/axis labels.
_LABEL_MIN_BUILD = 11669

# Minimum number of tokens in a valid "set pos <value>" command.
_SET_POS_MIN_PARTS = 3


@dataclass
class AxisInfo:
    """Identification data for one axis (peripheral) on a device."""

    resolution: int | None = None
    peripheral_id: int | None = None
    peripheral_serial: int | None = None
    hw_mod: bool | None = None
    conversion_table: object | None = None
    label: str | None = None


@dataclass
class Firmware:
    """Firmware version and build number for a device."""

    version: str
    build: str


@dataclass
class Protocol:
    """Extended protocol capabilities (FW7+)."""

    max_packets: int
    word_len: int
    max_sync_settings: int | None = None


@dataclass
class DeviceInfo:
    """Complete identification data for one device on the chain."""

    device_id: int
    serial: int
    firmware: Firmware
    axes: list[AxisInfo] = field(default_factory=list[AxisInfo])
    use_assumed_fw_version: bool = False
    hw_mod: bool | None = None
    label: str | None = None
    protocol: Protocol | None = None


async def run_identification_handshake(  # noqa: C901, PLR0912, PLR0915
    server: AsciiTcpServer,
    device_number: int,
    info: DeviceInfo,
) -> None:
    """Drive the server side of ZML's device.identify() handshake.

    Call this after wait_for_connection() and before handing control to
    MockDevice's dispatch loop (or your own test code).
    """
    d = device_number

    async def pop() -> Command:
        return parse_command(await server.read_line())

    async def reply(cmd: Command, data: str, status: str = "OK", axis: int | None = None) -> None:
        a = axis if axis is not None else cmd.axis
        await server.write_line(format_response(d, a, cmd.msg_id, status=status, data=data))

    async def reject(cmd: Command, axis: int | None = None) -> None:
        await reply(cmd, "BADCOMMAND", status="RJ", axis=axis)

    # --- Device-level identity ---
    cmd = await pop()
    await reply(cmd, str(info.device_id))       # get deviceid

    cmd = await pop()
    await reply(cmd, str(info.serial))           # get system.serial

    if not info.use_assumed_fw_version:
        cmd = await pop()
        await reply(cmd, info.firmware.version)  # get version

        cmd = await pop()
        await reply(cmd, info.firmware.build)    # get version.build

    cmd = await pop()                            # get device.hw.modified
    if info.hw_mod is None:
        await reject(cmd)
    else:
        await reply(cmd, "1" if info.hw_mod else "0")

    cmd = await pop()                            # get comm.command.packets.max
    if info.protocol is None:
        await reject(cmd)
    else:
        await reply(cmd, str(info.protocol.max_packets))

    cmd = await pop()                            # get comm.word.size.max
    if info.protocol is None:
        await reject(cmd)
    else:
        await reply(cmd, str(info.protocol.word_len))

    cmd = await pop()                            # get get.settings.max
    if info.protocol is None or info.protocol.max_sync_settings is None:
        await reject(cmd)
    else:
        await reply(cmd, str(info.protocol.max_sync_settings))

    # --- Axis count + per-axis identity ---
    cmd = await pop()                            # get system.axiscount
    await reply(cmd, str(len(info.axes)))

    for axis_num, axis_info in enumerate(info.axes, 1):
        cmd = await pop()                        # get resolution
        if axis_info.resolution is None:
            await reject(cmd)
        else:
            await reply(cmd, str(axis_info.resolution), axis=axis_num)

        cmd = await pop()                        # get peripheralid
        if axis_info.peripheral_id is None:
            # Note: original TS code uses axis 1 here regardless — replicate that
            await server.write_line(
                format_response(d, 1, cmd.msg_id, status="RJ", data="BADCOMMAND")
            )
        else:
            await reply(cmd, str(axis_info.peripheral_id), axis=axis_num)

            cmd = await pop()                    # get peripheral.serial
            if axis_info.peripheral_serial is None:
                await reject(cmd)
            else:
                serial = axis_info.peripheral_serial if axis_info.peripheral_id > 0 else 0
                await reply(cmd, str(serial), axis=axis_num)

            cmd = await pop()                    # get peripheral.hw.modified
            if axis_info.hw_mod is None:
                await reject(cmd)
            else:
                await reply(cmd, "1" if axis_info.hw_mod else "0", axis=axis_num)

    # --- Conversion tables for hw-modified axes ---
    for axis_num, axis_info in enumerate(info.axes, 1):
        needs_table = axis_info.hw_mod or (
            axis_info.peripheral_id is None and info.hw_mod
        )
        if needs_table:
            cmd = await pop()                    # storage axis get zaber.unit_conversions
            if axis_info.conversion_table:
                encoded = base64.b64encode(
                    json.dumps(axis_info.conversion_table).encode()
                ).decode()
                await reply(cmd, encoded, axis=axis_num)
            else:
                await server.write_line(
                    format_response(d, axis_num, cmd.msg_id, status="RJ", data="BADDATA")
                )

    # --- Labels (firmware build >= _LABEL_MIN_BUILD) ---
    if info.axes and int(info.firmware.build) >= _LABEL_MIN_BUILD:
        cmd = await pop()                        # storage get zaber.label (device)
        if info.label:
            encoded = base64.b64encode(info.label.encode()).decode()
            await reply(cmd, encoded)
        else:
            await server.write_line(
                format_response(d, cmd.axis, cmd.msg_id, status="RJ", data="BADDATA")
            )

        for _, axis_info in enumerate(info.axes, 1):
            if axis_info.peripheral_id is not None:
                cmd = await pop()                # storage axis get zaber.label
                if axis_info.label:
                    encoded = base64.b64encode(axis_info.label.encode()).decode()
                    await reply(cmd, encoded)
                else:
                    await server.write_line(
                        format_response(d, cmd.axis, cmd.msg_id, status="RJ", data="BADDATA")
                    )

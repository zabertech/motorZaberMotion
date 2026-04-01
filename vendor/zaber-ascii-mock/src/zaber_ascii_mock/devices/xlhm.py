"""DeviceInfo instances for the X-LHM linear stage (device ID 50081).

Ported from zaber-motion-lib/js/test-e2e/ascii/devices/xlhm.ts.

The X-LHM is a single-axis linear stage with no peripheral (the motor
is integrated). resolution=64 means 64 microsteps per native unit;
the actual µm-per-microstep conversion is resolved by ZML from the
device database using device_id=50081.
"""

from zaber_ascii_mock.identify import AxisInfo, DeviceInfo, Firmware

_AXIS = AxisInfo(resolution=64)

FW6 = DeviceInfo(
    device_id=50081,
    serial=1234,
    firmware=Firmware(version="6.28", build="1347"),
    axes=[_AXIS],
)

FW6_PROTO = DeviceInfo(
    device_id=50081,
    serial=1234,
    firmware=Firmware(version="6.99", build="0"),
    axes=[_AXIS],
    use_assumed_fw_version=True,
)

FW7 = DeviceInfo(
    device_id=50081,
    serial=1234,
    firmware=Firmware(version="7.33", build="14091"),
    axes=[_AXIS],
)

FW7_42 = DeviceInfo(
    device_id=50081,
    serial=1234,
    firmware=Firmware(version="7.42", build="18298"),
    axes=[_AXIS],
)

FW7_8 = DeviceInfo(
    device_id=50081,
    serial=1234,
    firmware=Firmware(version="7.08", build="3904"),
    axes=[_AXIS],
)

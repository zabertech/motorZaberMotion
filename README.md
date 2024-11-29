# motorZaberMotion

## About

EPICS motor driver for zaber devices.

motorZaberMotion is intended to be used as a submodule of [motor](https://github.com/epics-modules/motor).  When motorZaberMotion is built in the ``motor/modules`` directory, no manual configuration is needed.

motorZaberMotion can also be built outside of motor by copying it's ``EXAMPLE_RELEASE.local`` file to ``RELEASE.local`` and defining the paths to ``MOTOR`` and itself.

motorZaberMotion contains an example IOC that is built if ``CONFIG_SITE.local`` sets ``BUILD_IOCS = YES``.  The example IOC can be built outside of driver module.

## Documentation

This module makes use of [Zaber Motion Library](https://software.zaber.com/motion-library/docs) (ZML), which requires c++17 or greater. For this reason, any epics IOC which uses this motor module must also be compiled with at least c++17.

### ZML Device Database

Typically, ZML requires internet connectivity to identify devices: when identifying a device it will query a database service to retrieve important information such as device names, settings and conversion factors between device native units and real world units. If you would prefer that your IOC not be connected to the internet, this module exposes an ioc shell function for setting a local copy of the db (more on this in the ioc shell function section below).

### Officially supported systems and architectures:
While ZML itself supports windows, linux and macOS, this module only officially supports linux and maxOS, specifically:
- linux-aarch64
- linux-arm
- linux-x86_64
- darwin-aarch64
- darwin-x86

Even though Windows is not officially supported, we include our Windows dlls in `zaberMotionSupport/ZaberMotionCppSupport` just in case. If you would like us to add official support for Windows, please let us know!

Darwin users must add the following to the `configure/CONFIG_SITE` of any IOC which uses this module (example in `iocs/configure/CONFIG_SITE`):
```makefile
ifneq ($(filter darwin-%, $(T_A)),)
PRODDIR_RPATH_LDFLAGS_YES += $(PROD_DEPLIB_DIRS:%=-Wl,-rpath,%)
PRODDIR_LDFLAGS += $(PRODDIR_RPATH_LDFLAGS_$(LINKER_USE_RPATH))
endif
```

## Documentation

This module provides a single implementation of both `asynMotorController` and `asynMotorAxis`, which can be used for both rotary and linear devices.

### IOC Shell Functions

__ZaberMotionCreateController(port, numAxes, movingPollPeriod, idlePollPeriod, zaberPort, zaberDeviceNum)__
Instantiates `zaberController` with typical `asynMotorController` params and two zaber device-specific params:
- `zaberPort`: Address or serial port name of zaber device (prefixed with `tcp://` or `serial://`). A serial port can have multiple devices chained together.
- `zaberDeviceNum`: Number (1-indexed) of the controller device on a device chain. This number is stable unless the device chain is modified. It can be found using [Zaber Launcher](https://software.zaber.com/zaber-launcher/download).

__ZaberMotionSetDbPath(path)__
Allows user to specify the path to a local copy of the [Zaber device database](https://software.zaber.com/motion-library/docs/guides/device_db). This is only necessary if the IOC does not have access to the internet. Download link for current version of device database [here](https://www.zaber.com/software/device-database/devices-public.sqlite.lzma).
- `path`: Path to device db sqlite file.

### Implementation
This section is intended to clarify implementation-specific details and limitations for `zaberAxis` and `zaberController`

#### zaberAxis
Can control either a linear or rotary axis. The units for all motion commands are microns for linear devices and degrees for rotary. Please keep this in mind when configuring motor records.

__setPosition(position)__:
zaberAxis does not implement setPosition.

__move(position, relative, minVelocity, maxVelocity, acceleration)__:
Implemented as specified, except that `minVelocity` is ignored.

__moveVelocity(minVelocity, maxVelocity, acceleration)__:
Implemented as specified, except that `minVelocity` is ignored.

__home(minVelocity, maxVelocity, acceleration)__:
Note that this function ignores the `minVelocity`, `maxVelocity`, and `acceleration` parameters completely. The homing velocity of a zaber axis is defined as the lesser of `limit.approach.maxspeed` and `maxspeed` settings. If you wish to modify these settings, please do so using [Zaber Launcher](https://software.zaber.com/zaber-launcher/download).

__stop(acceleration)__:
Implemented as specified.

__poll(moving)__:
The following is an overview of which status flags are set during polling, and what the status values mean with relation to our devices:
- `DIRECTION`: not set and has no meaning for zaber axes -- this cannot be inferred from axis state.
- `DONE`: equivalent to `!zaber::motion::Axis::isBusy()` ([docs](https://software.zaber.com/motion-library/api/matlab/ascii/axis#isbusy)).
- `PLUS_LS`: 1 if `FE` flag is observed ([docs](https://www.zaber.com/protocol-manual?protocol=ASCII#topic_message_format_warning_flags))
- `HOMELS`: not set and has no meaning for zaber axes -- this cannot be inferred from axis state.
- `POSITION`: ie. closed-loop position enabled -- not set and has no meaning for zaber axes
- `SLIP_STALL`: set to true if either `FS` or `FP` flag is observed. ([docs](https://www.zaber.com/protocol-manual?protocol=ASCII#topic_message_format_warning_flags))
- `HOME`: not set and has no meaning for zaber axes -- this cannot be inferred from axis state.
- `PRESENT`: ie. encoder is present -- not set and has no meaning for zaber axes
- `PROBLEM`: 1 if ZML throws any type of exception, if any fault is observed, or if `WL` (unexpected limit trigger) flag is observed.
- `MOVING`: equivalent to `zaber::motion::Axis::isBusy()` ([docs](https://software.zaber.com/motion-library/api/matlab/ascii/axis#isbusy))
- `GAIN_SUPPORT`: ie. motor supports closed-loop position control -- not set and has no meaning for zaber axes
- `COMM_ERR`: 1 if ZML throws any type of exception.
- `MINUS_LS`: 1 if `FE` flag is observed ([docs](https://www.zaber.com/protocol-manual?protocol=ASCII#topic_message_format_warning_flags))
- `HOMED`: value returned from `zaber::motion::Axis::isHomed()` ([docs](https://software.zaber.com/motion-library/api/matlab/ascii/axis#ishomed))


### zaberController
Please note that profile moves have not been implemented for `zaberController`.

### Contact
For any comments or concerns, please contact us at `contact@zaber.com`

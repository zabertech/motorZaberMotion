# motorZaberMotion Releases

## __R2-2-0 (2026-08-04)__
R2-2-0 is a tagged release (git tag v2.2.0)

### Changes since R2-1-0

#### Bugfixes and Improvements
* Commit [8fb2265c](https://github.com/zabertech/motorZaberMotion/commit/8fb2265c4ec2f6069f9259e4bfe48e95e8ac11fb)
    * Fixes bug introduced by [variable MRES change](https://github.com/zabertech/motorZaberMotion/commit/94099d062cc42312cb7d1ee0eee2a08971f2b1ca).
        * The previous behaviour used the EPICS Motor MRES field in a non-standard way and caused an issue where the VAL field showed an incorrectly scaled value on startup.
        * The fix allows the user to pass an optional per-axis scaling factor to the ZaberMotionCreateController ioc shell function. Please see the IOC shell function documentation in the [README](../README.md) for more information.

#### New Features
* This version adds official support for profile moves.
    * Beta version of feature released in commit [52c066cb](https://github.com/zabertech/motorZaberMotion/commit/52c066cb9f63b7bd29c9f6aea8e2a294299ddbf9).
    * Digital output pulse behaviour fixed in commit [32118304](https://github.com/zabertech/motorZaberMotion/commit/3211830450064da9fd112f84cdb5a3aea25518ca).

## __R2-1-0 (2026-07-02)__
R2-1-0 is a tagged release (git tag v2.1.0)

### Changes since R2-0-0

#### Bugfixes and Improvements
* Commit [94099d06](https://github.com/zabertech/motorZaberMotion/commit/94099d062cc42312cb7d1ee0eee2a08971f2b1ca)
    * Adds support for variable MRES record values.
        * Users can now set their preferred decimal precision for axes and the driver will perform scaling before sending commands to the device.
        * Linear and rotary devices still use millimetres and degrees, respectively.

#### New Features
* Commit [52c066cb](https://github.com/zabertech/motorZaberMotion/commit/52c066cb9f63b7bd29c9f6aea8e2a294299ddbf9)
    * Adds support for profile moves (beta).
        * Note that this feature is still being tested and may be changed before it is officially released.

## __R2-0-0 (2026-03-09)__
R2-0-0 is a tagged release (git tag v2.0.0)

### Changes since R1-0

#### Modifications to existing features
* Commit [961c7ce](https://github.com/zabertech/motorZaberMotion/commit/961c7cedebb0eaa86fde46a109a2e6e8594c1d12): Update ZML Version
    * `ZaberMotionCppSupport` package will now be downloaded as part of the build process.
        * Details added to project [README](../README.md).
    * `ZaberMotionSetDbPath` IOC shell function no longer supports older versions of device database.
        * The new database is much more compact than the original, taking up MBs instead of GBs of storage.
        * Users who configured their IOC to work with a local copy of the device database must download an up-to-date version [here](https://software.zaber.com/motion-library/docs/guides/advanced/device_db).

## __R1-0-1 (2024-12-04)__
R1-0-1 is a tagged release (git tag v1.0.1)

#### Features
* This is the official first release of motorZaberMotion.
* Supports zaber controllers and integrated devices with FW version >= 7.25.
* Adds support for installing local copy of [Zaber device database](https://software.zaber.com/motion-library/docs/guides/advanced/device_db) for offline use.
* Examples for how to use various ioc shell functions added to `iocs/zaberMotionIOC/iocBoot/iocZaberMotion`.

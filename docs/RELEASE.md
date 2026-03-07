# motorZaberMotion Releases

## __R2-0-0 (2026-03-09)__
R2-0-0 is a tagged release (git tag v2.0.0)

### Changes since R1-0

#### Modifications to existing features
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

# Setup

This documentation is for developers who wish to contribute to this module.

## Updating ZML cpp support package
Make sure that aws cli is installed. Open `gulpfile.js` and change `ZML_VERSION` to version which you'd like to update to, then on the command line:
```
npm install
npx gulp update_support_package
```
This should download the relevant version of the support package. Please note that the support package was added in version `7.2.0`, so is not available for earlier versions.

## Building EPICS and Zaber Motor Module
### Building natively

__Build Steps__
- Follow instructions for installing and building EPICS base repository. Here are instructions for [Windows](https://docs.epics-controls.org/en/latest/getting-started/installation-windows.html) and [Linux and macOS](https://docs.epics-controls.org/en/latest/getting-started/installation-linux.html).
- Clone the EPICS [sequencer](https://github.com/epics-modules/sequencer), [asyn](https://github.com/epics-modules/asyn) and [motor](https://github.com/epics-modules/motor) modules with the following directory structure:
```
your-workfolder
|- EPICS
   |- epics-base
   |- support
      |- asyn
      |- sequencer
      |- motor
```

- install `libtirpc-dev` and `re2c` (check `images/epics_ubuntu.Dockerfile` for any other dependencies you might not have)
- In the motor repository, add this repo as a submodule at the path: `modules/motorZaberMotion`.

__Automated Configuration with NPM__
- Install node and npm
- Open the motorZaberMotion folder in a shell and run: `npm install` and then `npx gulp build`. This will perform the steps listed in explicit configuration steps below.
- You can run unit tests with `npx gulp test`

__Explicit Configuration Steps__
For documentation's sake, or for users who do not wish to install node and npm, below are the configuration and build steps performed by the `npx gulp build` command.

- In the asyn, motor and sequencer repositories, navigate to the file `./configure/RELEASE`, and set `SUPPORT` and `EPICS_BASE` to their respective global filesystem paths (ie. path to support and epics-base folders). Additionally, in the motor repo, set `ASYN` and `SNCSEQ` env variables to the global paths of `asyn` and `sequencer` folders.
- __Linux only__: In asyn repository, go to `./configure/CONFIG_SITE` and uncomment the line: `# TIRPC=YES`
- Open asyn in a shell and run `make`
- Open sequencer in a shell and run `make`
- In motor, open `modules/MakeFile` and add the line `SUBMODULES += motorZaberMotion` (make sure that all of other lines adding to the `SUBMODULES` var are commented out)
- Again in motor, run `make` -- this should build the epics motor library and the zaber motor driver library

### Building with Docker
- Grab either `registry.izaber.com/software-public/zaber-motion-epics/linux_amd64` or `registry.izaber.com/software-public/zaber-motion-epics/linux_arm64`
- Run the container and navigate to `~/EPICS/support/motor` and run `git submodule add https://gitlab.izaber.com/software-public/zaber-motion-epics.git modules/motorZaberMotion`
- Navigate to `modules/motorZaberMotion` and run `npm install` then `npx gulp build`
- Run tests with `npx gulp test`

## motorZaberMotion Example IOC

To build the example IOC, create a file `configure/CONFIG_SITE.local` and add:
```
BUILD_IOCS = YES
```
Then, run `make` in the motor module. Once the build is completed, you can configure your controllers and axes in [motor.cmd.zaber](iocs/zaberMotionIOC/iocBoot/iocZaberMotion/motor.cmd.zaber) and [motor.substitutions.zaber](iocs/zaberMotionIOC/iocBoot/iocZaberMotion/motor.substitutions.zaber), respectively.

To start the IOC, run the script which corresponds to your `EPICS_HOST_ARCH`:
```
./st.cmd.<EPICS_HOST_ARCH>
```

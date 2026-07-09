#include "zaberUtils.h"

#include <epicsExport.h>
#include <epicsStdio.h>
#include <iocsh.h>

#include <vector>
#include <string>

#include "zaberController.h"
#include "zaber/motion/library.h"

// Zaber Motion Controller

int zaberMotionCreateController(
        const char *portName,                   /* ZaberMotion Motor Asyn Port name */
        int numAxes,                            /* Number of axes this controller supports */
        int movingPollPeriod,                   /* Time to poll (msec) when an axis is in motion */
        int idlePollPeriod,                     /* Time to poll (msec) when an axis is idle. 0 for no polling */
        const char *zaberPort,                  /* Zaber Device TCP address or serial port name (prefixed with tcp:// or serial://) */
        int zaberDeviceNumber,                  /* Zaber Device number on the port (1-indexed) */
        const std::vector<double> &unitsPerStep /* Per-axis physical units (um linear / deg rotary) per motor-record step; default 1.0 */
) {
    zaberController *pController = new zaberController(portName, numAxes, static_cast<double>(movingPollPeriod), static_cast<double>(idlePollPeriod), zaberPort, zaberDeviceNumber, unitsPerStep);
    (void)pController;
    return (asynSuccess);
}

/* zaberMotionConfig */
static const iocshArg zaberMotionConfigArg0 = {"asyn motor port name", iocshArgString};
static const iocshArg zaberMotionConfigArg1 = {"number of axes", iocshArgInt};
static const iocshArg zaberMotionConfigArg2 = {"moving poll rate", iocshArgInt};
static const iocshArg zaberMotionConfigArg3 = {"idle poll rate", iocshArgInt};
static const iocshArg zaberMotionConfigArg4 = {"zaber serial/tcp port", iocshArgString};
static const iocshArg zaberMotionConfigArg5 = {"zaber device number", iocshArgInt};
static const iocshArg zaberMotionConfigArg6 = {"per-axis units-per-step (um linear / deg rotary), optional", iocshArgArgv};

static const iocshArg *const zaberMotionConfigArgs[7] = {&zaberMotionConfigArg0, &zaberMotionConfigArg1,
    &zaberMotionConfigArg2, &zaberMotionConfigArg3, &zaberMotionConfigArg4, &zaberMotionConfigArg5,
    &zaberMotionConfigArg6};

static const iocshFuncDef zaberMotionControllerDef = {"ZaberMotionCreateController", 7, zaberMotionConfigArgs};

static void zaberMotionCreateControllerCallFunc(const iocshArgBuf *args) {
    std::vector<double> unitsPerStep;
    for (int i = 1; i < args[6].aval.ac; i++) {
        try {
            unitsPerStep.push_back(std::stod(args[6].aval.av[i]));
        } catch (const std::exception &) {
            printf("ZaberMotionCreateController: could not parse units-per-step '%s' for axis %d; using 1.0\n",
                   args[6].aval.av[i], i - 1);
            unitsPerStep.push_back(1.0);
        }
    }
    zaberMotionCreateController(args[0].sval, args[1].ival, args[2].ival, args[3].ival, args[4].sval, args[5].ival, unitsPerStep);
}

// Create Profile

asynStatus ZaberControllerCreateProfile(
    const char *name, /* specify which controller by port name */
    int maxPoints     /* maximum number of profile points */
) {
  zaberController *pC;
  static const char *functionName = "ZaberControllerCreateProfile";

  pC = (zaberController*) findAsynPortDriver(name);
  if (!pC) {
    printf("%s:%s Error : Port %s not found\n",
           "ZaberController", functionName, name);
    return asynError;
  }
  pC->lock();
  pC->initializeProfile(maxPoints);
  pC->unlock();
  return asynSuccess;
}

static const iocshArg ZaberControllerCreateProfileArg0 = {"Controller port name", iocshArgString};
static const iocshArg ZaberControllerCreateProfileArg1 = {"Max points", iocshArgInt};
static const iocshArg * const ZaberControllerCreateProfileArgs[] = {&ZaberControllerCreateProfileArg0,
                                                                    &ZaberControllerCreateProfileArg1};
static const iocshFuncDef configZaberControllerProfile = {"ZaberControllerCreateProfile", 2, ZaberControllerCreateProfileArgs};

static void configZaberControllerProfileCallFunc(const iocshArgBuf *args) {
  ZaberControllerCreateProfile(args[0].sval, args[1].ival);
}


// Zaber Motion Device DB

int zaberMotionSetDbPath(const char *dbPath) {
    try {
        zml::Library::setDeviceDbSource(zml::DeviceDbSourceType::FILE, dbPath);
    } catch (const std::runtime_error &e) {
        printf("Zaber Motion Error: Failed to set device DB source to %s\n", dbPath);
        printf("\tFailed with error: %s\n\n", e.what());
        return (asynError);
    }
    return (asynSuccess);
}

static const iocshArg zaberMotionDbConfigArg0 = {"zaber device db path", iocshArgString};
static const iocshArg *const zaberMotionDbConfigArgs[1] = {&zaberMotionDbConfigArg0};
static const iocshFuncDef zaberMotionDbPathDef = {"ZaberMotionSetDbPath", 1, zaberMotionDbConfigArgs};

static void zaberMotionSetDbPathCallFunc(const iocshArgBuf *args) {
    zaberMotionSetDbPath(args[0].sval);
}

/* Registration */

static void zaberMotionAsynRegister(void) {
    iocshRegister(&zaberMotionControllerDef, zaberMotionCreateControllerCallFunc);
    iocshRegister(&zaberMotionDbPathDef, zaberMotionSetDbPathCallFunc);
    iocshRegister(&configZaberControllerProfile, configZaberControllerProfileCallFunc);
}

extern "C" {
epicsExportRegistrar(zaberMotionAsynRegister);
}

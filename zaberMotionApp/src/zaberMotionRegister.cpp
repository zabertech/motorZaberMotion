#include "zaberUtils.h"

#include <epicsExport.h>
#include <epicsStdio.h>
#include <iocsh.h>

#include "zaberController.h"
#include "zaber/motion/library.h"

// Zaber Motion Controller

int zaberMotionCreateController(
        const char *portName,   /* ZaberMotion Motor Asyn Port name */
        int numAxes,            /* Number of axes this controller supports */
        int movingPollPeriod,   /* Time to poll (msec) when an axis is in motion */
        int idlePollPeriod,     /* Time to poll (msec) when an axis is idle. 0 for no polling */
        const char *zaberPort,  /* Zaber Device TCP address or serial port name (prefixed with tcp:// or serial://) */
        int zaberDeviceNumber   /* Zaber Device number on the port (1-indexed) */
    )
{
    zaberController *pController = new zaberController(portName, numAxes, static_cast<double>(movingPollPeriod), static_cast<double>(idlePollPeriod), zaberPort, zaberDeviceNumber);
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

static const iocshArg *const zaberMotionConfigArgs[6] = {&zaberMotionConfigArg0, &zaberMotionConfigArg1,
    &zaberMotionConfigArg2, &zaberMotionConfigArg3, &zaberMotionConfigArg4, &zaberMotionConfigArg5};

static const iocshFuncDef zaberMotionControllerDef = {"ZaberMotionCreateController", 6, zaberMotionConfigArgs};

static void zaberMotionCreateControllerCallFunc(const iocshArgBuf *args) {
    zaberMotionCreateController(args[0].sval, args[1].ival, args[2].ival, args[3].ival, args[4].sval, args[5].ival);
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
}

extern "C" {
epicsExportRegistrar(zaberMotionAsynRegister);
}

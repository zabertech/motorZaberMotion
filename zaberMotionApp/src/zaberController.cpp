#include "zaberController.h"
#include <epicsExport.h>
#include <epicsThread.h>
#include <iocsh.h>
#include <iostream>
#include <zaber/motion/dto/ascii/device_identity.h>

#include "zaberConnectionManager.h"

#define PROFILE_MOVE_ERR_STR "Zaber Motion Error: Profile moves not implemented for zaberController\n"

/**
 * Creates a new zaberController object.
 * \param[in] portName          The name of the asyn port that will be created for this driver
 * \param[in] numAxes           The number of axes that this controller supports
 * \param[in] movingPollPeriod  The time between polls when any axis is moving
 * \param[in] idlePollPeriod    The time between polls when no axis is moving
 * \param[in] devicePort        The tcp or serial port that the zaber device is connected to
 * \param[in] deviceNumber      The number of the device on the port (1-indexed)
 */
zaberController::zaberController(const char *portName, int numAxes, double movingPollPeriod, double idlePollPeriod, const char *devicePort, int deviceNumber) :
        asynMotorController(portName, numAxes, 0, 0, 0, ASYN_CANBLOCK | ASYN_MULTIDEVICE, 1 /* autoconnect */, 0, 0) {
    try {
        connection_ = zaberConnectionManager::singleton().tryGetConnection(devicePort);
        device_ = connection_->getDevice(deviceNumber);
        device_.identify();
        connection_->setDisconnectedCallback([](const std::shared_ptr<zml::exceptions::MotionLibException>&) {
            printf("zaberController: disconnected\n");
        });
    } catch(const std::exception &e) {
        printf("zaberController: Connection failed: %s\n", e.what());
    }
    for(int i = 0; i < numAxes; i++) {
        new zaberAxis(this, i);
    }
    startPoller(static_cast<double>(movingPollPeriod) / 1000.0, static_cast<double>(idlePollPeriod) / 1000.0, 2);
    
    // just in case you need to clean up resources
    // epicsAtExit(zaberController::shutdown, this);
}

void zaberController::report(FILE *fp, int level) {
    zml::ascii::DeviceIdentity identity = device_.getIdentity();

    fprintf(fp, "Zaber Motion Controller: %s\n", this->portName);
    fprintf(fp, "  Firmware: %s\n", identity.getFirmwareVersion().toString().c_str());
    fprintf(fp, "  Device: %s\n", identity.getName().c_str());
    fprintf(fp, "  Serial Number: %d\n", identity.getSerialNumber());
    fprintf(fp, "  Num Axes: %d\n", numAxes_);

    asynMotorController::report(fp, level);
 }

zml::ascii::Axis zaberController::getDeviceAxis(int axisNo) { return device_.getAxis(axisNo + 1); }

zaberAxis *zaberController::getAxis(asynUser *pasynUser) {
    return static_cast<zaberAxis *>(asynMotorController::getAxis(pasynUser));
}

zaberAxis *zaberController::getAxis(int axisNo) {
    return static_cast<zaberAxis*>(asynMotorController::getAxis(axisNo));
}

asynStatus zaberController::initializeProfile(size_t maxPoints) {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, PROFILE_MOVE_ERR_STR);
    return asynError;
}

asynStatus zaberController::buildProfile() {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, PROFILE_MOVE_ERR_STR);
    return asynError;
}

asynStatus zaberController::executeProfile() {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, PROFILE_MOVE_ERR_STR);
    return asynError;
}

asynStatus zaberController::abortProfile() {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, PROFILE_MOVE_ERR_STR);
    return asynError;
}

asynStatus zaberController::readbackProfile() {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, PROFILE_MOVE_ERR_STR);
    return asynError;
}

/* Code for iocsh registration */

int zaberMotionCreateController(
        const char *portName, /* ZaberMotion Motor Asyn Port name */
        int numAxes, /* Number of axes this controller supports */
        int movingPollPeriod, /* Time to poll (msec) when an axis is in motion */
        int idlePollPeriod, /* Time to poll (msec) when an axis is idle. 0 for no polling */
        const char *zaberPort, /* Zaber Device TCP address or serial port name */
        int zaberDeviceNumber /* Zaber Device number on the port (1-indexed) */
    )
{
    // int priority = epicsThreadPriorityMedium;
    // int stackSize = epicsThreadGetStackSize(epicsThreadStackMedium);
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

static void zaberMotionAsynRegister(void) {
    iocshRegister(&zaberMotionControllerDef, zaberMotionCreateControllerCallFunc);
}

extern "C" {
epicsExportRegistrar(zaberMotionAsynRegister);
}

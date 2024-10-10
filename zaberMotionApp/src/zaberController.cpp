#include "zaberController.h"

#include <iostream>

#include <epicsExport.h>
#include <epicsThread.h>
#include <iocsh.h>

#include <zaber/motion/dto/ascii/device_identity.h>

int zaberController::zaberTotalControllerNumber = 0;

zaberController::zaberController(const char *portName, int numAxes, const char *serialPort, int deviceId, int priority, int stackSize, int extMotorParams) :
        asynMotorController(portName, numAxes, 0, 0, 0, ASYN_CANBLOCK | ASYN_MULTIDEVICE, 1 /* autoconnect */, 0, 0) {
    (void)serialPort;
    (void)priority;
    (void)stackSize;
    (void)extMotorParams;

    try {
        std::cout << "zaberController: constructor called" << std::endl;
        connection_ = zaber::motion::ascii::Connection::openNetworkShare(std::string(serialPort), 11421);
        device_ = connection_.getDevice(1);
        device_.identify();
        std::cout << "Device identity: " << device_.getIdentity().toString() << std::endl;
        connection_.setDisconnectedCallback([](const std::shared_ptr<zaber::motion::exceptions::MotionLibException>&) {
            std::cout << "zaberController: Connection lost" << std::endl;
        });
    } catch (const std::exception &e) {
        std::cerr << "zaberController: Connection failed: " << e.what() << std::endl;
    }
    for (int i  = 0; i < numAxes; i++) {
        std::cout << "initializing axis" << std::endl;
        new zaberAxis(this, i);
    }
}

void zaberController::report(FILE *fp, int level) {
    std::cout << "zaberController: Report called" << std::endl;
}

void zaberController::zaberPoller() {
    std::cout << "zaberController: Poller called" << std::endl;
}

void zaberController::callPoller(void *) {
    std::cout << "zaberController: Poller called" << std::endl;
}

void zaberController::shutdown() {
    std::cout << "zaberController: Shutdown called" << std::endl;
}

zaber::motion::ascii::Axis zaberController::getDeviceAxis(int axisNo) {
    return device_.getAxis(axisNo + 1);
}

zaberAxis* zaberController::getAxis(asynUser *pasynUser) {
    return static_cast<zaberAxis*>(asynMotorController::getAxis(pasynUser));
}

zaberAxis* zaberController::getAxis(int axisNo) {
    return static_cast<zaberAxis*>(asynMotorController::getAxis(axisNo));
}

/* Profile Moves */
asynStatus zaberController::initializeProfile(size_t maxPoints) {
    std::cout << "zaberController: Profile initialized" << std::endl;
    return asynSuccess;
}

asynStatus zaberController::buildProfile() {
    std::cout << "zaberController: Profile built" << std::endl;
    return asynSuccess;
}

asynStatus zaberController::executeProfile() {
    std::cout << "zaberController: Profile executed" << std::endl;
    return asynSuccess;
}

asynStatus zaberController::abortProfile() {
    std::cout << "zaberController: Profile aborted" << std::endl;
    return asynSuccess;
}

asynStatus zaberController::readbackProfile() {
    std::cout << "zaberController: Profile readback" << std::endl;
    return asynSuccess;
}


/* Code for iocsh registration */

int zaberMotionCreateController(
        const char *portName, /* ZaberMotion Motor Asyn Port name */
        int numAxes, /* Number of axes this controller supports */
        const char *serialPortName, /* ZaberMotion Serial Asyn Port name */
        int deviceId, /* Device ID */
        int movingPollPeriod, /* Time to poll (msec) when an axis is in motion */
        int idlePollPeriod) /* Time to poll (msec) when an axis is idle. 0 for no polling */
{
    int priority = epicsThreadPriorityMedium;
    int stackSize = epicsThreadGetStackSize(epicsThreadStackMedium);
    zaberController *pController = new zaberController(portName, numAxes, serialPortName, deviceId, priority, stackSize, 0);
    pController->startPoller((double)movingPollPeriod, (double)idlePollPeriod, 2);
    return (asynSuccess);
}

/* zaberMotionConfig */
static const iocshArg zaberMotionConfigArg0 = { "asyn motor port name", iocshArgString };
static const iocshArg zaberMotionConfigArg1 = { "number of axes", iocshArgInt };
static const iocshArg zaberMotionConfigArg2 = { "serial/tcp port name", iocshArgString };
static const iocshArg zaberMotionConfigArg3 = { "device id", iocshArgInt };
static const iocshArg zaberMotionConfigArg4 = { "moving poll rate", iocshArgInt };
static const iocshArg zaberMotionConfigArg5 = { "idle poll rate", iocshArgInt };

static const iocshArg *const zaberMotionConfigArgs[6] = {
    &zaberMotionConfigArg0,
    &zaberMotionConfigArg1,
    &zaberMotionConfigArg2,
    &zaberMotionConfigArg3,
    &zaberMotionConfigArg4,
    &zaberMotionConfigArg5
};

static const iocshFuncDef zaberMotionControllerDef = { "ZaberMotionCreateController", 6, zaberMotionConfigArgs };

static void zaberMotionCreateControllerCallFunc(const iocshArgBuf *args) {
    zaberMotionCreateController(args[0].sval, args[1].ival, args[2].sval, args[3].ival, args[4].ival, args[5].ival);
}

static void zaberMotionAsynRegister(void) {
    iocshRegister(&zaberMotionControllerDef, zaberMotionCreateControllerCallFunc);
}

extern "C" {
epicsExportRegistrar(zaberMotionAsynRegister);
}

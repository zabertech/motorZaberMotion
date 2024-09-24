#include "zaberBaseController.h"
#include "zaberAxis.h"

#include <iostream>

#include <iocsh.h>
#include <epicsExport.h>
#include <epicsThread.h>

int zaberBaseController::zaberTotalControllerNumber = 0;

zaberBaseController::zaberBaseController(const char *portName, int numAxes, const char* serialPort, int deviceId, int priority, int stackSize, int extMotorParams)
    : asynMotorController(portName, numAxes, 0, 0, 0, ASYN_CANBLOCK | ASYN_MULTIDEVICE, 1 /* autoconnect */, priority, stackSize)
{
    (void)serialPort;
    std::cout << "zaberBaseController: constructor called" << std::endl;
    zaberAxis *pAxis = new zaberAxis(this, 0);
    connection_ = zaber::motion::ascii::Connection::openNetworkShare(std::string(serialPort), 11421);
}

void zaberBaseController::report(FILE *fp, int level) {
    std::cout << "zaberBaseController: Report called" << std::endl;
}

void zaberBaseController::zaberPoller() {
    std::cout << "zaberBaseController: Poller called" << std::endl;
}

asynStatus zaberBaseController::startPoller(double movingPollPeriod, double idlePollPeriod, int forcedFastPolls) {
    std::cout << "zaberBaseController: Poller started" << std::endl;
    return asynSuccess;
}

void zaberBaseController::callPoller(void*) {
    std::cout << "zaberBaseController: Poller called" << std::endl;
}

void zaberBaseController::shutdown() {
    std::cout << "zaberBaseController: Shutdown called" << std::endl;
}

zaber::motion::ascii::Axis zaberBaseController::getDeviceAxis(int axisNo) {
    return device_.getAxis(axisNo + 1);
}

// zaberAxis* zaberBaseController::getAxis(asynUser *pasynUser) {
//     return static_cast<zaberAxis*>(asynMotorController::getAxis(pasynUser));
// }

// zaberAxis* zaberBaseController::getAxis(int axisNo) {
//     return static_cast<zaberAxis*>(asynMotorController::getAxis(axisNo));
// }

/* Code for iocsh registration */

int zaberMotionCreateController(
    const char *portName,      /* ZaberMotion Motor Asyn Port name */
    int numAxes,               /* Number of axes this controller supports */
    const char *serialPortName,/* ZaberMotion Serial Asyn Port name */
    int deviceId,              /* Device ID */
    int movingPollPeriod,      /* Time to poll (msec) when an axis is in motion */
    int idlePollPeriod)        /* Time to poll (msec) when an axis is idle. 0 for no polling */
{
    int priority = epicsThreadPriorityMedium;
    int stackSize = epicsThreadGetStackSize(epicsThreadStackMedium);
    zaberBaseController *pController = new zaberBaseController(portName, numAxes, serialPortName, deviceId, priority, stackSize, 0);
    pController->startPoller((double)movingPollPeriod, (double)idlePollPeriod, 10);
    return(asynSuccess);
}

/* zaberMotionConfig */
static const iocshArg zaberMotionConfigArg0 = {"asyn motor port name", iocshArgString};
static const iocshArg zaberMotionConfigArg1 = {"number of axes", iocshArgInt};
static const iocshArg zaberMotionConfigArg2 = {"serial/tcp port name", iocshArgString};
static const iocshArg zaberMotionConfigArg3 = {"device id", iocshArgInt};
static const iocshArg zaberMotionConfigArg4 = {"moving poll rate", iocshArgInt};
static const iocshArg zaberMotionConfigArg5 = {"idle poll rate", iocshArgInt};

static const iocshArg * const zaberMotionConfigArgs[6] = {
    &zaberMotionConfigArg0,
    &zaberMotionConfigArg1,
    &zaberMotionConfigArg2,
    &zaberMotionConfigArg3,
    &zaberMotionConfigArg4,
    &zaberMotionConfigArg5
};

static const iocshFuncDef zaberMotionControllerDef = {"ZaberMotionCreateController", 6, zaberMotionConfigArgs };

static void zaberMotionCreateControllerCallFunc(const iocshArgBuf *args) {
    zaberMotionCreateController(args[0].sval, args[1].ival, args[2].sval, args[3].ival, args[4].ival, args[5].ival);
}

static void zaberMotionAsynRegister(void) {
    iocshRegister(&zaberMotionControllerDef, zaberMotionCreateControllerCallFunc);
}

extern "C" {
epicsExportRegistrar(zaberMotionAsynRegister);
}

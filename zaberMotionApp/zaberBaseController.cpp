#include "zaberBaseController.h"

#include <iostream>

int zaberBaseController::zaberTotalControllerNumber = 0;

zaberBaseController::zaberBaseController(const char *portName, int numAxes, int deviceId, int priority, int stackSize, int extMotorParams)
    : asynMotorController(portName, numAxes, 0, 0, 0, ASYN_CANBLOCK | ASYN_MULTIDEVICE, 1 /* autoconnect */, priority, stackSize) {

    connection_ = zaber::motion::ascii::Connection::openNetworkShare("martin-pi", 11421);
}

void zaberBaseController::report(FILE *fp, int level) {
    std::cout << "Report called" << std::endl;
}

void zaberBaseController::zaberPoller() {
    std::cout << "Poller called" << std::endl;
}

asynStatus zaberBaseController::startPoller(double movingPollPeriod, double idlePollPeriod, int forcedFastPolls) {
    std::cout << "Poller started" << std::endl;
    return asynSuccess;
}

void zaberBaseController::callPoller(void*) {
    std::cout << "Poller called" << std::endl;
}

void zaberBaseController::shutdown() {
    std::cout << "Shutdown called" << std::endl;
}

zaber::motion::ascii::Axis zaberBaseController::getDeviceAxis(int axisNo) {
    return device_.getAxis(axisNo);
}

/* Code for iocsh registration */

extern "C"
{

/* omsMAXnetConfig */
static const iocshArg omsMAXnetConfigArg0 = {"asyn motor port name", iocshArgString};
static const iocshArg omsMAXnetConfigArg1 = {"number of axes", iocshArgInt};
static const iocshArg omsMAXnetConfigArg2 = {"asyn serial/tcp port name", iocshArgString};
static const iocshArg omsMAXnetConfigArg3 = {"moving poll rate", iocshArgInt};
static const iocshArg omsMAXnetConfigArg4 = {"idle poll rate", iocshArgInt};
static const iocshArg omsMAXnetConfigArg5 = {"initstring", iocshArgString};
static const iocshArg * const omsMAXnetConfigArgs[6] = {&omsMAXnetConfigArg0,
                                                  &omsMAXnetConfigArg1,
                                                  &omsMAXnetConfigArg2,
                                                  &omsMAXnetConfigArg3,
                                                  &omsMAXnetConfigArg4,
                                                  &omsMAXnetConfigArg5 };
static const iocshFuncDef configOmsMAXnet = {"omsMAXnetConfig", 6, omsMAXnetConfigArgs};
static void configOmsMAXnetCallFunc(const iocshArgBuf *args)
{
    omsMAXnetConfig(args[0].sval, args[1].ival, args[2].sval, args[3].ival, args[4].ival, args[5].sval);
}

static void OmsMAXnetAsynRegister(void)
{
    iocshRegister(&configOmsMAXnet,     configOmsMAXnetCallFunc);
}

epicsExportRegistrar(OmsMAXnetAsynRegister);

}

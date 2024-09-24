#ifndef ZABER_BASE_CONTROLLER_H
#define ZABER_BASE_CONTROLLER_H

#include <asynMotorController.h>

#include <zaber/motion/ascii/connection.h>
#include <zaber/motion/ascii/device.h>
#include <zaber/motion/ascii/axis.h>

class zaberAxis;

class zaberBaseController : public asynMotorController {
public:
    zaberBaseController(const char *portName, int numAxes, const char* serialPort, int deviceId, int priority, int stackSize, int extMotorParams);
    void report(FILE *fp, int level) override;
    void zaberPoller();
    asynStatus startPoller(double movingPollPeriod, double idlePollPeriod, int forcedFastPolls) override;
    static void callPoller(void*);
    static void callShutdown(void *ptr){((zaberBaseController*)ptr)->shutdown();};
    void shutdown();
private:
    friend zaberAxis;

    static int zaberTotalControllerNumber;
    zaber::motion::ascii::Connection connection_;
    zaber::motion::ascii::Device device_;

    zaber::motion::ascii::Axis getDeviceAxis(int axisNo);
};

#endif
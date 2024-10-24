#ifndef ZABER_BASE_CONTROLLER_H
#define ZABER_BASE_CONTROLLER_H

#include <memory>

#include "zaberAxis.h"
#include <asynMotorController.h>
#include <zaber/motion/ascii/axis.h>
#include <zaber/motion/ascii/connection.h>
#include <zaber/motion/ascii/device.h>

class zaberAxis;

using zml = zaber::motion;

class epicsShareClass zaberController : public asynMotorController {
    public:
    zaberController(const char *portName, const char *serialPort, int numAxes,
        double movingPollPeriod, double idlePollPeriod);
    void report(FILE *fp, int level) override;

    zaberAxis *getAxis(asynUser *pasynUser);
    zaberAxis *getAxis(int axisNo);

    // TODO: Decide whether or not to implement profile moves
    // you could set up PVT in this way, but the system seems to not allow users
    // to add velocity or time information
    asynStatus initializeProfile(size_t maxPoints) override;
    asynStatus buildProfile() override;
    asynStatus executeProfile() override;
    asynStatus abortProfile() override;
    asynStatus readbackProfile() override;

    private:
    std::shared_ptr<zml::ascii::Connection> connection_;
    zml::ascii::Device device_;

    friend zaberAxis;
    zml::ascii::Axis getDeviceAxis(int axisNo);
};

#endif
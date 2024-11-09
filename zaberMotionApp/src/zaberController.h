#ifndef ZABER_BASE_CONTROLLER_H
#define ZABER_BASE_CONTROLLER_H

#include <memory>

#include "zaberAxis.h"
#include <asynMotorController.h>
#include <zaber/motion/ascii/axis.h>
#include <zaber/motion/ascii/connection.h>
#include <zaber/motion/ascii/device.h>

class zaberAxis;

namespace zml = zaber::motion;

class epicsShareClass zaberController : public asynMotorController {
    public:
    zaberController(const char *portName, int numAxes, double movingPollPeriod, double idlePollPeriod, const char *devicePort, int deviceNumber);
    void report(FILE *fp, int level) override;

    zaberAxis *getAxis(asynUser *pasynUser) override;
    zaberAxis *getAxis(int axisNo) override;

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

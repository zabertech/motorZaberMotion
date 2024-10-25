#ifndef ZABER_AXIS_H
#define ZABER_AXIS_H

#include <asynMotorAxis.h>
#include <zaber/motion/ascii/axis.h>

namespace zml = zaber::motion;

class zaberController;

class epicsShareClass zaberAxis : public asynMotorAxis {
    public:
    zaberAxis(zaberController *pC, int axisNo);
    ~zaberAxis();
    void report(FILE *fp, int details) override;
    asynStatus move(double position, int relative, double minVelocity, double maxVelocity, double acceleration) override;
    asynStatus moveVelocity(double minVelocity, double maxVelocity, double acceleration) override;
    asynStatus home(double minVelocity, double maxVelocity, double acceleration, int forwards) override;
    asynStatus stop(double acceleration) override;
    asynStatus poll(bool *moving) override;

    private:
    zaberController *pC_;
    zml::ascii::Axis axis_;
    zml::Units lengthUnit_;
    zml::Units velocityUnit_;
    zml::Units accelUnit_;

    asynStatus doAbsoluteMove(double position, double velocity, double acceleration);
    asynStatus doRelativeMove(double distance, double velocity, double acceleration);
};

#endif // ZABER_AXIS_H
#ifndef ZABER_AXIS_H
#define ZABER_AXIS_H

#include <zaber/motion/ascii/axis.h>
#include <asynMotorAxis.h>

class zaberController;

class epicsShareClass zaberAxis : public asynMotorAxis {
public:
    zaberAxis(zaberController *pC, int axisNo);
    ~zaberAxis();
    asynStatus move(double position, int relative, double minVelocity, double maxVelocity, double acceleration) override;
    asynStatus moveVelocity(double minVelocity, double maxVelocity, double acceleration) override;
    asynStatus home(double minVelocity, double maxVelocity, double acceleration, int forwards) override;
    asynStatus stop(double acceleration) override;
    asynStatus poll(bool *moving) override;

private:
    zaberController *pC_;
    zaber::motion::ascii::Axis axis_;
};

#endif // ZABER_AXIS_H
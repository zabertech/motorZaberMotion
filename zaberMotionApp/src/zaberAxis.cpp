#include "zaberAxis.h"

zaberAxis::zaberAxis(zaberBaseController *pC, int axisNo) : asynMotorAxis(pC, axisNo) {
    axis_ = pC->getDeviceAxis(axisNo);
}

asynStatus zaberAxis::move(double position, int relative, double minVelocity, double maxVelocity, double acceleration) {
    (void)relative;
    (void)minVelocity;
    (void)maxVelocity;
    (void)acceleration;

    axis_.moveAbsolute(position);
    return asynSuccess;
}

asynStatus zaberAxis::moveVelocity(double minVelocity, double maxVelocity, double acceleration) {
    (void)minVelocity;
    (void)acceleration;

    axis_.moveVelocity(maxVelocity);
    return asynSuccess;
}

asynStatus zaberAxis::home(double minVelocity, double maxVelocity, double acceleration, int forwards) {
    (void)minVelocity;
    (void)maxVelocity;
    (void)acceleration;
    (void)forwards;

    axis_.home();
    return asynSuccess;
}

asynStatus zaberAxis::stop(double acceleration) {
    (void)acceleration;

    axis_.stop();
    return asynSuccess;
}

asynStatus zaberAxis::poll(bool *moving) {
    *moving = axis_.isBusy();
    return asynSuccess;
}

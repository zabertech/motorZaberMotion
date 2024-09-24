#include "zaberAxis.h"
#include <iostream>

zaberAxis::zaberAxis(zaberBaseController *pC, int axisNo) : asynMotorAxis(pC, axisNo) {
    std::cout << "zaberAxis: constructor called" << std::endl;
    pC_ = pC;
    axis_ = pC->getDeviceAxis(axisNo);
}

zaberAxis::~zaberAxis() { }

asynStatus zaberAxis::move(double position, int relative, double minVelocity, double maxVelocity, double acceleration) {
    (void)relative;
    (void)minVelocity;
    (void)maxVelocity;
    (void)acceleration;

    std::cout << "zaberAxis::move with position: " << position << std::endl;
    axis_.moveAbsolute(position);
    return asynSuccess;
}

asynStatus zaberAxis::moveVelocity(double minVelocity, double maxVelocity, double acceleration) {
    (void)minVelocity;
    (void)acceleration;

    std::cout << "zaberAxis::moveVelocity with maxVelocity: " << maxVelocity << std::endl;
    axis_.moveVelocity(maxVelocity);
    return asynSuccess;
}

asynStatus zaberAxis::home(double minVelocity, double maxVelocity, double acceleration, int forwards) {
    (void)minVelocity;
    (void)maxVelocity;
    (void)acceleration;
    (void)forwards;

    std::cout << "zaberAxis::home" << std::endl;
    axis_.home();
    return asynSuccess;
}

asynStatus zaberAxis::stop(double acceleration) {
    (void)acceleration;

    std::cout << "zaberAxis::stop" << std::endl;
    axis_.stop();
    return asynSuccess;
}

asynStatus zaberAxis::poll(bool *moving) {
    *moving = axis_.isBusy();
    return asynSuccess;
}

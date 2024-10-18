#include "zaberAxis.h"
#include "zaberController.h"
#include "zaberUtils.h"
#include <iostream>
#include <string>
#include <vector>
#include <zaber/motion/ascii/axis_settings.h>
#include <zaber/motion/dto/ascii/axis_type.h>
#include <zaber/motion/dto/ascii/get_axis_setting.h>
#include <zaber/motion/units.h>

using namespace zaber::motion;

const char *ZABER_MAX_SPEED = "maxspeed";
const char *ZABER_ACCEL = "acceleration";

zaberAxis::zaberAxis(zaberController *pC, int axisNo) :
        asynMotorAxis(pC, axisNo) {
    pC_ = pC;
    axis_ = pC->getDeviceAxis(axisNo);

    switch(axis_.getAxisType()) {
        case ascii::AxisType::LINEAR:
            lengthUnit_ = zaber::motion::Units::LENGTH_MICROMETRES;
            velocityUnit_ = zaber::motion::Units::VELOCITY_MICROMETRES_PER_SECOND;
            accelUnit_ = zaber::motion::Units::ACCELERATION_MICROMETRES_PER_SECOND_SQUARED;
            break;
        case ascii::AxisType::ROTARY:
            lengthUnit_ = zaber::motion::Units::ANGLE_DEGREES;
            velocityUnit_ = zaber::motion::Units::ANGULAR_VELOCITY_DEGREES_PER_SECOND;
            accelUnit_ = zaber::motion::Units::ANGULAR_ACCELERATION_DEGREES_PER_SECOND_SQUARED;
            break;
        default:
            std::cerr << "Zaber Motion Error: Unsupported axis type -- " << ascii::AxisType_toString(axis_.getAxisType()) << std::endl;
            std::cerr << "Please refer to the documentation on supported axis types" << std::endl;
            break;
    }
}

zaberAxis::~zaberAxis() {}

asynStatus zaberAxis::move(double position, int relative, double minVelocity, double maxVelocity, double acceleration) {
    (void)minVelocity;

    asynStatus status = asynSuccess;
    if(relative) {
        status = doRelativeMove(position, maxVelocity, acceleration);
    } else {
        status = doAbsoluteMove(position, maxVelocity, acceleration);
    }
    pC_->wakeupPoller();
    return status;
}

asynStatus zaberAxis::moveVelocity(double minVelocity, double maxVelocity, double acceleration) {
    (void)minVelocity;
    axis_.getAxisType();
    std::function<asynStatus()> action = [this, maxVelocity, acceleration]() {
        std::cout << "zaberAxis::moveVelocity with maxVelocity: " << maxVelocity << " " << getUnitLongName(velocityUnit_) << std::endl;
        ascii::Axis::MoveVelocityOptions options{
            .acceleration = acceleration,
            .accelerationUnit = accelUnit_};
        axis_.moveVelocity(maxVelocity, velocityUnit_, options);
        return asynSuccess;
    };
    asynStatus status = zaber::epics::performAction(action);
    pC_->wakeupPoller();
    return status;
}

asynStatus zaberAxis::home(double minVelocity, double maxVelocity, double acceleration, int forwards) {
    (void)minVelocity;
    (void)forwards;

    std::function<asynStatus()> action = [this, minVelocity, maxVelocity, acceleration]() {
        std::cout << "zaberAxis::home" << std::endl;
        axis_.home(false);
        return asynSuccess;
    };
    asynStatus status = zaber::epics::performAction(action);
    pC_->wakeupPoller();
    return status;
}

asynStatus zaberAxis::stop(double acceleration) {
    std::function<asynStatus()> action = [this, acceleration]() {
        std::cout << "zaberAxis::stop" << std::endl;
        checkUpdateAccel(acceleration);
        axis_.stop();
        return asynSuccess;
    };
    asynStatus status = zaber::epics::performAction(action);
    pC_->wakeupPoller();
    return status;
}

/**
 * Here are the axis status flags which can be set (comment for internal review only):
 *
 * int motorStatusDirection_;
 * int motorStatusDone_;
 * int motorStatusHighLimit_;
 * int motorStatusAtHome_;
 * int motorStatusSlip_;
 * int motorStatusPowerOn_;
 * int motorStatusFollowingError_;
 * int motorStatusHome_;
 * int motorStatusHasEncoder_; include ??
 * int motorStatusProblem_;
 * int motorStatusMoving_;
 * int motorStatusGainSupport_;
 * int motorStatusCommsError_;
 * int motorStatusLowLimit_;
 * int motorStatusHomed_;
 */
asynStatus zaberAxis::poll(bool *moving) {
    std::function<asynStatus(void)> action = [this, &moving]() {
        *moving = axis_.isBusy();
        double d = axis_.getPosition(lengthUnit_);
        setDoubleParam(pC_->motorPosition_, d);

        int i = static_cast<int>(!(*moving));
        setIntegerParam(pC_->motorStatusDone_, i);

        i = static_cast<int>(axis_.isHomed());
        setIntegerParam(pC_->motorStatusAtHome_, i); // TODO: this probably needs some more elaborate logic
        setIntegerParam(pC_->motorStatusHome_, i);
        setIntegerParam(pC_->motorStatusHomed_, i);
        setIntegerParam(pC_->motorStatusMoving_, static_cast<int>(*moving));
        setIntegerParam(pC_->motorStatusCommsError_, 0);
        setIntegerParam(pC_->motorStatusProblem_, 0);
        return asynStatus::asynSuccess;
    };
    std::function<void()> onError = [this]() {
        setIntegerParam(pC_->motorStatusCommsError_, 1);
        setIntegerParam(pC_->motorStatusProblem_, 1);
    };

    asynStatus status = zaber::epics::performAction(action, onError);
    callParamCallbacks();
    return status;
}

/* Private member functions */

asynStatus zaberAxis::doAbsoluteMove(double position, double velocity, double acceleration) {
    std::function<asynStatus()> action = [this, position, velocity, acceleration]() {
        std::cout << "zaberAxis" << axisNo_ << "::moveAbs with position: " << position << std::endl;
        ascii::Axis::MoveAbsoluteOptions options{
            .waitUntilIdle = false,
            .velocity = velocity,
            .velocityUnit = velocityUnit_,
            .acceleration = acceleration,
            .accelerationUnit = accelUnit_};
        axis_.moveAbsolute(position, lengthUnit_, options);
        return asynSuccess;
    };
    return zaber::epics::performAction(action);
}

asynStatus zaberAxis::doRelativeMove(double distance, double velocity, double acceleration) {
    std::function<asynStatus()> action = [this, distance, velocity, acceleration]() {
        std::cout << "zaberAxis" << axisNo_ << "::moveRel with distance: " << distance << std::endl;
        ascii::Axis::MoveRelativeOptions options{
            .waitUntilIdle = false,
            .velocity = velocity,
            .velocityUnit = velocityUnit_,
            .acceleration = acceleration,
            .accelerationUnit = accelUnit_};
        axis_.moveRelative(distance, lengthUnit_, options);
        return asynSuccess;
    };
    return zaber::epics::performAction(action);
}

asynStatus zaberAxis::checkUpdateAccel(double acceleration) {
    std::function<asynStatus()> action = [this, acceleration]() {
        double result = axis_.getSettings().get(ZABER_ACCEL, accelUnit_);
        if(result != acceleration) {
            axis_.getSettings().set(ZABER_ACCEL, acceleration);
        }
        return asynSuccess;
    };
    return zaber::epics::performAction(action);
}


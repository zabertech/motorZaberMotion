#include "zaberAxis.h"
#include "zaberController.h"
#include "zaberUtils.h"
#include <iostream>
#include <string>
#include <vector>
#include <zaber/motion/ascii/axis_settings.h>
#include <zaber/motion/dto/ascii/get_axis_setting.h>
#include <zaber/motion/units.h>

using namespace zaber::motion;

const char *ZABER_MAX_SPEED = "maxspeed";
const char *ZABER_ACCEL = "acceleration";

zaberAxis::zaberAxis(zaberController *pC, int axisNo) :
        asynMotorAxis(pC, axisNo) {
    std::cout << "zaberAxis: constructor called" << std::endl;
    pC_ = pC;
    axis_ = pC->getDeviceAxis(axisNo);
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
    std::function<asynStatus()> action = [this, maxVelocity, acceleration]() {
        std::cout << "zaberAxis::moveVelocity with maxVelocity: " << maxVelocity << std::endl;
        ascii::Axis::MoveVelocityOptions options{
            .acceleration = acceleration,
            .accelerationUnit = Units::ACCELERATION_MILLIMETRES_PER_SECOND_SQUARED};
        axis_.moveVelocity(maxVelocity, Units::VELOCITY_MILLIMETRES_PER_SECOND, options);
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
        // checkUpdateSpeedParams(maxVelocity, acceleration);
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
 * int motorStatusHasEncoder_;
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
        double d = axis_.getPosition(zaber::motion::Units::LENGTH_MILLIMETRES);
        setDoubleParam(pC_->motorPosition_, d);

        int i = static_cast<int>(!(*moving));
        setIntegerParam(pC_->motorStatusDone_, i);

        i = static_cast<int>(axis_.isHomed());
        setIntegerParam(pC_->motorStatusAtHome_, i);
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
            .velocityUnit = Units::VELOCITY_MILLIMETRES_PER_SECOND,
            .acceleration = acceleration,
            .accelerationUnit = Units::ACCELERATION_MILLIMETRES_PER_SECOND_SQUARED};
        axis_.moveAbsolute(position, zaber::motion::Units::LENGTH_MILLIMETRES, options);
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
            .velocityUnit = Units::VELOCITY_MILLIMETRES_PER_SECOND,
            .acceleration = acceleration,
            .accelerationUnit = Units::ACCELERATION_MILLIMETRES_PER_SECOND_SQUARED};
        axis_.moveRelative(distance, zaber::motion::Units::LENGTH_MILLIMETRES, options);
        return asynSuccess;
    };
    return zaber::epics::performAction(action);
}

asynStatus zaberAxis::checkUpdateAccel(double acceleration) {
    std::function<asynStatus()> action = [this, acceleration]() {
        double result = axis_.getSettings().get(ZABER_ACCEL, Units::ACCELERATION_MILLIMETRES_PER_SECOND_SQUARED);
        if(result != acceleration) {
            axis_.getSettings().set(ZABER_ACCEL, acceleration);
        }
        return asynSuccess;
    };
    return zaber::epics::performAction(action);
}

asynStatus zaberAxis::checkUpdateSpeedParams(double maxVelocity, double acceleration) {
    static bool (*maxSpeedPred)(const ascii::GetAxisSettingResult &) = [](const ascii::GetAxisSettingResult &r) { return r.setting == ZABER_MAX_SPEED; };
    static bool (*accelPred)(const ascii::GetAxisSettingResult &) = [](const ascii::GetAxisSettingResult &r) { return r.setting == ZABER_ACCEL; };
    
    std::function<asynStatus()> action = [this, maxVelocity, acceleration]() {
        ascii::GetAxisSetting getMaxSpeed(ZABER_MAX_SPEED, Units::VELOCITY_MILLIMETRES_PER_SECOND);
        ascii::GetAxisSetting getAccel(ZABER_ACCEL, Units::ACCELERATION_MILLIMETRES_PER_SECOND_SQUARED);

        std::vector<ascii::GetAxisSettingResult> result = axis_.getSettings().getMany(getMaxSpeed, getAccel);
        const auto &maxSpeedIt = std::find_if(result.begin(), result.end(), maxSpeedPred);
        if(maxSpeedIt != result.end() && (*maxSpeedIt).value != maxVelocity) {
            axis_.getSettings().set(ZABER_MAX_SPEED, maxVelocity);
        }
        const auto &accelIt = std::find_if(result.begin(), result.end(), accelPred);
        if(accelIt != result.end() && (*accelIt).value != acceleration) {
            axis_.getSettings().set(ZABER_ACCEL, acceleration);
        }
        return asynSuccess;
    };
    return zaber::epics::performAction(action);
}

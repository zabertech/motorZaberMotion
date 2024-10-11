#include "zaberAxis.h"

#include <iostream>
#include <vector>

#include <zaber/motion/ascii/axis_settings.h>
#include <zaber/motion/dto/ascii/get_axis_setting.h>
#include <zaber/motion/units.h>

#include "zaberController.h"
#include "zaberUtils.h"

using namespace zaber::motion;

zaberAxis::zaberAxis(zaberController *pC, int axisNo) : asynMotorAxis(pC, axisNo) {
    std::cout << "zaberAxis: constructor called" << std::endl;
    pC_ = pC;
    axis_ = pC->getDeviceAxis(axisNo);
}

zaberAxis::~zaberAxis() {}

asynStatus zaberAxis::move(double position, int relative, double minVelocity, double maxVelocity, double acceleration) {
    (void)relative;
    (void)minVelocity;
    (void)maxVelocity;
    (void)acceleration;

    try {
        std::cout << "zaberAxis" << axisNo_ << "::move with position: " << position << std::endl;
        axis_.moveAbsolute(position, zaber::motion::Units::LENGTH_MILLIMETRES, false);
        pC_->wakeupPoller();
    } catch (const std::exception &e) {
        std::cerr << "zaberAxis::move failed: " << e.what() << std::endl;
        return asynError;
    }
    return asynSuccess;
}

asynStatus zaberAxis::moveVelocity(double minVelocity, double maxVelocity, double acceleration) {
    (void)minVelocity;
    (void)acceleration;

    try {
        std::cout << "zaberAxis::moveVelocity with maxVelocity: " << maxVelocity << std::endl;
        axis_.moveVelocity(maxVelocity);
    } catch (const std::exception &e) {
        std::cerr << "zaberAxis::moveVelocity failed: " << e.what() << std::endl;
    }

    return asynSuccess;
}

asynStatus zaberAxis::home(double minVelocity, double maxVelocity, double acceleration, int forwards) {
    (void)forwards;

    std::cout << "zaberAxis::home" << std::endl;
    std::vector<ascii::GetAxisSettingResult> result =
            axis_.getSettings().getMany(ascii::GetAxisSetting("maxspeed", Units::VELOCITY_MILLIMETRES_PER_SECOND),
                    ascii::GetAxisSetting("acceleration", Units::ACCELERATION_MILLIMETRES_PER_SECOND_SQUARED));
    const auto &maxSpeedIt = std::find_if(
            result.begin(), result.end(), [](const ascii::GetAxisSettingResult &r) { return r.setting == "maxspeed"; });
    if (maxSpeedIt != result.end() && (*maxSpeedIt).value != maxVelocity) {
        axis_.getSettings().set("maxspeed", maxVelocity);
    }
    const auto &accelIt = std::find_if(result.begin(), result.end(),
            [](const ascii::GetAxisSettingResult &r) { return r.setting == "acceleration"; });
    if (accelIt != result.end() && (*accelIt).value != acceleration) {
        axis_.getSettings().set("acceleration", acceleration);
    }
    axis_.home();
    return asynSuccess;
}

asynStatus zaberAxis::stop(double acceleration) {
    (void)acceleration;

    std::cout << "zaberAxis::stop" << std::endl;
    axis_.stop();
    return asynSuccess;
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

    asynStatus status = performAction(action, onError);
    std::cout << "zaberAxis" << axisNo_ << "::poll -- moving: " << axis_.isBusy() << std::endl;
    std::cout << "zaberAxis" << axisNo_ << "::poll -- position: " << axis_.getPosition(zaber::motion::Units::LENGTH_MILLIMETRES)
              << std::endl;
    callParamCallbacks();
    return status;
}

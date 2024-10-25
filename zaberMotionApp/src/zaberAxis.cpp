#include "zaberAxis.h"
#include "zaberController.h"
#include "zaberUtils.h"
#include <iostream>
#include <string>
#include <vector>

#include <zaber/motion/dto/ascii/axis_identity.h>
#include <zaber/motion/ascii/axis_settings.h>
#include <zaber/motion/dto/ascii/axis_type.h>
#include <zaber/motion/dto/ascii/get_axis_setting.h>
#include <zaber/motion/units.h>

const char *ZABER_MAX_SPEED = "maxspeed";
const char *ZABER_DECEL = "decelonly";

zaberAxis::zaberAxis(zaberController *pC, int axisNo) :
        asynMotorAxis(pC, axisNo) {
    pC_ = pC;
    axis_ = pC->getDeviceAxis(axisNo);

    switch(axis_.getAxisType()) {
        case zml::ascii::AxisType::LINEAR:
            lengthUnit_ = zml::Units::LENGTH_MICROMETRES;
            velocityUnit_ = zml::Units::VELOCITY_MICROMETRES_PER_SECOND;
            accelUnit_ = zml::Units::ACCELERATION_MICROMETRES_PER_SECOND_SQUARED;
            break;
        case zml::ascii::AxisType::ROTARY:
            lengthUnit_ = zml::Units::ANGLE_DEGREES;
            velocityUnit_ = zml::Units::ANGULAR_VELOCITY_DEGREES_PER_SECOND;
            accelUnit_ = zml::Units::ANGULAR_ACCELERATION_DEGREES_PER_SECOND_SQUARED;
            break;
        default:
            std::cerr << "Zaber Motion Error: Unsupported axis type -- " << zml::ascii::AxisType_toString(axis_.getAxisType()) << std::endl;
            std::cerr << "Please refer to the documentation on supported axis types" << std::endl;
            break;
    }
}

zaberAxis::~zaberAxis() {}

void zaberAxis::report(FILE *fp, int details) {
    fprintf(fp, "  Zaber Motion Axis: %d\n", axisNo_);
    zml::ascii::AxisIdentity identity = axis_.getIdentity();
    if (!identity.getIsPeripheral()) {
        fprintf(fp, "    Peripheral Name: %s\n", identity.peripheralName.c_str());
        fprintf(fp, "    Type: %s\n", zml::ascii::AxisType_toString(identity.axisType).c_str());
    }
    if (details > 0) {
        double velocity, position, acceleration;
        pC_->lock();
        pC_->getDoubleParam(pC_->motorVelocity_, &velocity);
        pC_->getDoubleParam(pC_->motorPosition_, &position);
        pC_->getDoubleParam(pC_->motorAccel_, &acceleration);
        pC_->unlock();

        fprintf(fp, "    Position: %f\n", position);
        fprintf(fp, "    Velocity: %f\n", velocity);
        fprintf(fp, "    Acceleration: %f\n", acceleration);
    }
}

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
        std::cout << "zaberAxis::moveVelocity with maxVelocity: " << maxVelocity << " " << getUnitLongName(velocityUnit_) << std::endl;
        zml::ascii::Axis::MoveVelocityOptions options{
            .acceleration = acceleration,
            .accelerationUnit = accelUnit_};
        axis_.moveVelocity(maxVelocity, velocityUnit_, options);
        return asynSuccess;
    };
    asynStatus status = zaber::epics::handleException(action);
    pC_->wakeupPoller();
    return status;
}

asynStatus zaberAxis::home(double minVelocity, double maxVelocity, double acceleration, int forwards) {
    (void)minVelocity;
    (void)forwards;

    std::function<asynStatus()> action = [this, minVelocity, maxVelocity, acceleration]() {
        std::cout << "zaberAxis::home" << std::endl;
        // update velocity here??
        axis_.home(false);
        return asynSuccess;
    };
    asynStatus status = zaber::epics::handleException(action);
    pC_->wakeupPoller();
    return status;
}

asynStatus zaberAxis::stop(double acceleration) {
    std::function<asynStatus()> action = [this, acceleration]() {
        std::cout << "zaberAxis::stop" << std::endl;
        zml::ascii::Axis::MoveVelocityOptions options{
            .acceleration = acceleration,
            .accelerationUnit = accelUnit_};
        axis_.moveVelocity(0., velocityUnit_, options);
        return asynSuccess;
    };
    asynStatus status = zaber::epics::handleException(action);
    pC_->wakeupPoller();
    return status;
}

/**
 * Here are the axis status flags which can be set (comment for internal review only):
 *
 * int motorStatusDirection_;
 * int motorStatusSlip_;
 * int motorStatusPowerOn_;
 * int motorStatusFollowingError_;
 * int motorStatusHasEncoder_; include ??
 * int motorStatusGainSupport_;
 * int motorStatusLowLimit_;
 * int motorStatusHighLimit_;
 */
asynStatus zaberAxis::poll(bool *moving) {
    std::function<asynStatus(void)> action = [this, &moving]() {
        *moving = axis_.isBusy();

        // check flags here and convert to appropriate error in EPICS

        double pos = axis_.getPosition(lengthUnit_);
        setDoubleParam(pC_->motorPosition_, pos);

        int i = static_cast<int>(!(*moving));
        setIntegerParam(pC_->motorStatusDone_, i);

        i = static_cast<int>(axis_.isHomed());
        // AtHome and Home cannot really be inferred from axis data
        // setIntegerParam(pC_->motorStatusAtHome_, pos == 0. && static_cast<bool>(i));
        // setIntegerParam(pC_->motorStatusHome_, pos == 0. && static_cast<bool>(i));
        
        setIntegerParam(pC_->motorStatusHomed_, i);


        setIntegerParam(pC_->motorStatusHighLimit_, 0);
        setIntegerParam(pC_->motorStatusLowLimit_, 0);

        setIntegerParam(pC_->motorStatusMoving_, static_cast<int>(*moving));
        setIntegerParam(pC_->motorStatusCommsError_, 0);
        setIntegerParam(pC_->motorStatusProblem_, 0);
        return asynStatus::asynSuccess;
    };
    std::function<void()> onError = [this]() {
        setIntegerParam(pC_->motorStatusCommsError_, 1);
        setIntegerParam(pC_->motorStatusProblem_, 1);
    };

    asynStatus status = zaber::epics::handleException(action, onError);
    callParamCallbacks();
    return status;
}

/* Private member functions */

asynStatus zaberAxis::doAbsoluteMove(double position, double velocity, double acceleration) {
    std::function<asynStatus()> action = [this, position, velocity, acceleration]() {
        std::cout << "zaberAxis" << axisNo_ << "::moveAbs with position: " << position;
        std:: cout << " velocity: " << velocity << " " << getUnitLongName(velocityUnit_) << std::endl;
        zml::ascii::Axis::MoveAbsoluteOptions options{
            .waitUntilIdle = false,
            .velocity = velocity,
            .velocityUnit = velocityUnit_,
            .acceleration = acceleration,
            .accelerationUnit = accelUnit_};
        axis_.moveAbsolute(position, lengthUnit_, options);
        return asynSuccess;
    };
    return zaber::epics::handleException(action);
}

asynStatus zaberAxis::doRelativeMove(double distance, double velocity, double acceleration) {
    std::function<asynStatus()> action = [this, distance, velocity, acceleration]() {
        std::cout << "zaberAxis" << axisNo_ << "::moveRel with distance: " << distance;
        std::cout << " velocity: " << velocity << " " << getUnitLongName(velocityUnit_) << std::endl;
        zml::ascii::Axis::MoveRelativeOptions options{
            .waitUntilIdle = false,
            .velocity = velocity,
            .velocityUnit = velocityUnit_,
            .acceleration = acceleration,
            .accelerationUnit = accelUnit_};
        axis_.moveRelative(distance, lengthUnit_, options);
        return asynSuccess;
    };
    return zaber::epics::handleException(action);
}



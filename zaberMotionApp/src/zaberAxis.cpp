#include "zaberAxis.h"

#include <iostream>
#include <string>
#include <vector>

#include <zaber/motion/dto/ascii/axis_identity.h>
#include <zaber/motion/ascii/warning_flags.h>
#include <zaber/motion/ascii/warnings.h>
#include <zaber/motion/ascii/axis_settings.h>
#include <zaber/motion/dto/ascii/axis_type.h>
#include <zaber/motion/dto/ascii/get_axis_setting.h>
#include <zaber/motion/units.h>

#include "zaberController.h"
#include "zaberUtils.h"

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
            printf("Zaber Motion Error: Unsupported axis type -- %s\n", zml::ascii::AxisType_toString(axis_.getAxisType()).c_str());
            printf("Please refer to the documentation on supported axis types\n");
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

        double pos = axis_.getPosition(lengthUnit_);
        setDoubleParam(pC_->motorPosition_, pos);

        int i = static_cast<int>(!(*moving));
        setIntegerParam(pC_->motorStatusDone_, i);
        setIntegerParam(pC_->motorStatusMoving_, static_cast<int>(*moving));

        i = static_cast<int>(axis_.isHomed());
        setIntegerParam(pC_->motorStatusHomed_, i);
        
        // AtHome and Home cannot be inferred from axis state
        setIntegerParam(pC_->motorStatusAtHome_, 0);
        setIntegerParam(pC_->motorStatusHome_, 0);
        
        // no communication error
        setIntegerParam(pC_->motorStatusCommsError_, 0);
        setIntegerParam(pC_->motorStatusProblem_, 0);

        // check all flags
        zml::ascii::Warnings warnings = axis_.getWarnings();
        std::unordered_set<std::string> flags = warnings.getFlags();
        checkAllFlags(flags);
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

void zaberAxis::checkAllFlags(std::unordered_set<std::string> flags) {
    checkFlag(flags, zml::ascii::warning_flags::CRITICAL_SYSTEM_ERROR, "Critical system error");
    checkFlag(flags, zml::ascii::warning_flags::PERIPHERAL_NOT_SUPPORTED, "Peripheral not supported");
    checkFlag(flags, zml::ascii::warning_flags::PERIPHERAL_INACTIVE, "Peripheral inactive");
    checkFlag(flags, zml::ascii::warning_flags::HARDWARE_EMERGENCY_STOP, "Hardware emergency stop");
    checkFlag(flags, zml::ascii::warning_flags::OVERVOLTAGE_OR_UNDERVOLTAGE, "Overvoltage or undervoltage");
    checkFlag(flags, zml::ascii::warning_flags::DRIVER_DISABLED_NO_FAULT, "Driver disabled, no fault");
    checkFlag(flags, zml::ascii::warning_flags::CURRENT_INRUSH_ERROR, "Current inrush error");
    checkFlag(flags, zml::ascii::warning_flags::MOTOR_TEMPERATURE_ERROR, "Motor temperature error");
    checkFlag(flags, zml::ascii::warning_flags::DRIVER_DISABLED, "Driver disabled");
    checkFlag(flags, zml::ascii::warning_flags::ENCODER_ERROR, "Encoder error");
    checkFlag(flags, zml::ascii::warning_flags::INDEX_ERROR, "Index error");
    checkFlag(flags, zml::ascii::warning_flags::ANALOG_ENCODER_SYNC_ERROR, "Analog encoder sync error");
    checkFlag(flags, zml::ascii::warning_flags::OVERDRIVE_LIMIT_EXCEEDED, "Overdrive limit exceeded");
    checkFlag(flags, zml::ascii::warning_flags::STALLED_AND_STOPPED, "Stalled and stopped");
    checkFlag(flags, zml::ascii::warning_flags::STREAM_BOUNDS_ERROR, "Stream bounds error");
    checkFlag(flags, zml::ascii::warning_flags::INTERPOLATED_PATH_DEVIATION, "Interpolated path deviation");
    checkFlag(flags, zml::ascii::warning_flags::LIMIT_ERROR, "Limit error");
    checkFlag(flags, zml::ascii::warning_flags::EXCESSIVE_TWIST, "Excessive twist");
    checkFlag(flags, zml::ascii::warning_flags::UNEXPECTED_LIMIT_TRIGGER, "Unexpected limit trigger");
    checkFlag(flags, zml::ascii::warning_flags::VOLTAGE_OUT_OF_RANGE, "Voltage out of range");
    checkFlag(flags, zml::ascii::warning_flags::CONTROLLER_TEMPERATURE_HIGH, "Controller temperature high");
    checkFlag(flags, zml::ascii::warning_flags::STALLED_WITH_RECOVERY, "Stalled with recovery");
    checkFlag(flags, zml::ascii::warning_flags::DISPLACED_WHEN_STATIONARY, "Displaced when stationary");
    checkFlag(flags, zml::ascii::warning_flags::INVALID_CALIBRATION_TYPE, "Invalid calibration type");
    checkFlag(flags, zml::ascii::warning_flags::NO_REFERENCE_POSITION, "No reference position");
    checkFlag(flags, zml::ascii::warning_flags::DEVICE_NOT_HOMED, "Device not homed");
    checkFlag(flags, zml::ascii::warning_flags::MANUAL_CONTROL, "Manual control");
    checkFlag(flags, zml::ascii::warning_flags::MOVEMENT_INTERRUPTED, "Movement interrupted");
    checkFlag(flags, zml::ascii::warning_flags::STREAM_DISCONTINUITY, "Stream discontinuity");
    checkFlag(flags, zml::ascii::warning_flags::VALUE_ROUNDED, "Value rounded");
    checkFlag(flags, zml::ascii::warning_flags::VALUE_TRUNCATED, "Value truncated");
    checkFlag(flags, zml::ascii::warning_flags::SETTING_UPDATE_PENDING, "Setting update pending");
}

void zaberAxis::checkFlag(std::unordered_set<std::string> flags, const std::string &flag, const std::string &message) {
    if(flags.find(flag) != flags.end()) {
        printf("%s\n", message.c_str());
    }
}
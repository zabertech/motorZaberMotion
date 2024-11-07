#include "zaberAxis.h"
#include "zaberController.h"
#include "zaberUtils.h"

#include <iostream>
#include <string>
#include <vector>

#include <zaber/motion/ascii/axis_settings.h>
#include <zaber/motion/ascii/warning_flags.h>
#include <zaber/motion/ascii/warnings.h>
#include <zaber/motion/dto/ascii/axis_identity.h>
#include <zaber/motion/dto/ascii/axis_type.h>
#include <zaber/motion/dto/ascii/get_axis_setting.h>
#include <zaber/motion/units.h>

#define ZABER_MAX_SPEED = "maxspeed";
#define ZABER_ACCEL = "accel";

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
    if(!identity.getIsPeripheral()) {
        fprintf(fp, "    Peripheral Name: %s\n", identity.peripheralName.c_str());
        fprintf(fp, "    Type: %s\n", zml::ascii::AxisType_toString(identity.axisType).c_str());
    }
    if(details > 0) {
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

/**
 * Perform relative or absolute move.
 * 
 * @param position The target position.
 * @param relative If true, the move is relative.
 * @param minVelocity This parameter is ignored.
 * @param maxVelocity Corresponds to `maxspeed` setting in Zaber ASCII protocol.
 * @param acceleration Corresponds to `accel` setting in Zaber ASCII protocol.
 */
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

/**
 * Move the axis at velocity.
 * 
 * @param minVelocity This parameter is ignored.
 * @param maxVelocity Corresponds to maxspeed setting in Zaber ASCII protocol.
 * @param acceleration Corresponds to accel setting in Zaber ASCII protocol.
 */
asynStatus zaberAxis::moveVelocity(double minVelocity, double maxVelocity, double acceleration) {
    (void)minVelocity;

    std::function<asynStatus()> action = [this, maxVelocity, acceleration]() {
        zml::ascii::Axis::MoveVelocityOptions options{
            .acceleration = acceleration,
            .accelerationUnit = accelUnit_};
        axis_.moveVelocity(maxVelocity, velocityUnit_, options);
        return asynSuccess;
    };
    asynStatus status = zaber::epics::handleException(pC_->pasynUserSelf, action);
    pC_->wakeupPoller();
    return status;
}

/**
 * Home the axis.
 * 
 * Note that this function ignores the minVelocity, maxVelocity, and acceleration parameters.
 * The homing velocity of a zaber axis is defined as the lesser of `limit.approach.maxspeed` and
 * `maxspeed` settings. If you wish to modify these settings, please do so using Zaber Launcher.
 */
asynStatus zaberAxis::home(double minVelocity, double maxVelocity, double acceleration, int forwards) {
    (void)minVelocity;
    (void)maxVelocity;
    (void)acceleration;
    (void)forwards;

    std::function<asynStatus()> action = [this]() {
        axis_.home(false);
        return asynSuccess;
    };
    asynStatus status = zaber::epics::handleException(pC_->pasynUserSelf, action);
    pC_->wakeupPoller();
    return status;
}

/**
 * Stop the axis with given deceleration.
 * 
 * @param acceleration Corresponds to accel setting in Zaber ASCII protocol.
 */
asynStatus zaberAxis::stop(double acceleration) {
    std::function<asynStatus()> action = [this, acceleration]() {
        zml::ascii::Axis::MoveVelocityOptions options{
            .acceleration = acceleration,
            .accelerationUnit = accelUnit_};
        axis_.moveVelocity(0., velocityUnit_, options);
        return asynSuccess;
    };
    asynStatus status = zaber::epics::handleException(pC_->pasynUserSelf, action);
    pC_->wakeupPoller();
    return status;
}

/**
 * Here are the axis status flags which can be set (comment for internal review only):
 * 
 * motorStatusDirection_      -- DIRECTION: last raw direction; (0:Negative, 1:Positive)
 * motorStatusDone_           -- DONE: motion is complete.
 * motorStatusHighLimit_      -- PLUS_LS: plus limit switch has been hit.
 * motorStatusAtHome_         -- HOMELS: state of the home limit switch.
 * motorStatusSlip_           -- Unused
 * motorStatusPowerOn_        -- POSITION: closed-loop position control is enabled.
 * motorStatusFollowingError_ -- SLIP_STALL: Slip/Stall detected (eg. fatal following error)
 * motorStatusHome_           -- HOME: if at home position.
 * motorStatusHasEncoder_     -- PRESENT: encoder is present.
 * motorStatusProblem_        -- PROBLEM: driver stopped polling, or hardware problem
 * motorStatusMoving_         -- MOVING: non-zero velocity present.
 * motorStatusGainSupport_    -- GAIN_SUPPORT: motor supports closed-loop position control.
 * motorStatusCommsError_     -- COMM_ERR: Controller communication error.
 * motorStatusLowLimit_       -- MINUS_LS: minus limit switch has been hit.
 * motorStatusHomed_          -- HOMED: the motor has been homed.
 */
asynStatus zaberAxis::poll(bool *moving) {
    std::function<asynStatus(void)> action = [this, &moving]() {
        *moving = axis_.isBusy();
        setIntegerParam(pC_->motorStatusDone_, static_cast<int>(!*moving));
        setIntegerParam(pC_->motorStatusMoving_, static_cast<int>(*moving));
        setIntegerParam(pC_->motorStatusHomed_, static_cast<int>(axis_.isHomed()));

        double pos = axis_.getPosition(lengthUnit_);
        setDoubleParam(pC_->motorPosition_, pos);
        setIntegerParam(pC_->motorStatusCommsError_, 0);

        // AtHome and Home cannot be inferred from axis state
        setIntegerParam(pC_->motorStatusAtHome_, 0);
        setIntegerParam(pC_->motorStatusHome_, 0);

        // check and clear all flags
        zml::ascii::Warnings warnings = axis_.getWarnings();
        std::unordered_set<std::string> flags = warnings.getFlags();
        warnings.clearFlags();

        if(checkAllFlags(flags)) {
            setIntegerParam(pC_->motorStatusProblem_, 1);
            return asynError;
        }

        setIntegerParam(pC_->motorStatusProblem_, 0);
        return asynSuccess;
    };
    std::function<void()> onError = [this]() {
        setIntegerParam(pC_->motorStatusCommsError_, 1);
        setIntegerParam(pC_->motorStatusProblem_, 1);
    };

    asynStatus status = zaber::epics::handleException(pC_->pasynUserSelf, action, onError);
    callParamCallbacks();
    return status;
}

/* Private member functions */

asynStatus zaberAxis::doAbsoluteMove(double position, double velocity, double acceleration) {
    std::function<asynStatus()> action = [this, position, velocity, acceleration]() {
        zml::ascii::Axis::MoveAbsoluteOptions options{
            .waitUntilIdle = false,
            .velocity = velocity,
            .velocityUnit = velocityUnit_,
            .acceleration = acceleration,
            .accelerationUnit = accelUnit_};
        axis_.moveAbsolute(position, lengthUnit_, options);
        return asynSuccess;
    };
    return zaber::epics::handleException(pC_->pasynUserSelf, action);
}

asynStatus zaberAxis::doRelativeMove(double distance, double velocity, double acceleration) {
    std::function<asynStatus()> action = [this, distance, velocity, acceleration]() {
        zml::ascii::Axis::MoveRelativeOptions options{
            .waitUntilIdle = false,
            .velocity = velocity,
            .velocityUnit = velocityUnit_,
            .acceleration = acceleration,
            .accelerationUnit = accelUnit_};
        axis_.moveRelative(distance, lengthUnit_, options);
        return asynSuccess;
    };
    return zaber::epics::handleException(pC_->pasynUserSelf, action);
}

/**
 * Here are the axis status flags which can be set (comment for internal review only):
 * 
 * motorStatusDirection_      -- DIRECTION: last raw direction; (0:Negative, 1:Positive)
 * motorStatusDone_           -- DONE: motion is complete.
 * motorStatusHighLimit_      -- PLUS_LS: plus limit switch has been hit.
 * motorStatusAtHome_         -- HOMELS: state of the home limit switch.
 * motorStatusSlip_           -- Unused
 * motorStatusPowerOn_        -- POSITION: closed-loop position control is enabled.
 * motorStatusFollowingError_ -- SLIP_STALL: Slip/Stall detected (eg. fatal following error)
 * motorStatusHome_           -- HOME: if at home position.
 * motorStatusHasEncoder_     -- PRESENT: encoder is present.
 * motorStatusProblem_        -- PROBLEM: driver stopped polling, or hardware problem
 * motorStatusMoving_         -- MOVING: non-zero velocity present.
 * motorStatusGainSupport_    -- GAIN_SUPPORT: motor supports closed-loop position control.
 * motorStatusCommsError_     -- COMM_ERR: Controller communication error.
 * motorStatusLowLimit_       -- MINUS_LS: minus limit switch has been hit.
 * motorStatusHomed_          -- HOMED: the motor has been homed.
 */
bool zaberAxis::checkAllFlags(std::unordered_set<std::string> flags) {
    bool fault = false;
    fault |= checkFlag(flags, zml::ascii::warning_flags::CRITICAL_SYSTEM_ERROR, "Critical system error");
    fault |= checkFlag(flags, zml::ascii::warning_flags::PERIPHERAL_NOT_SUPPORTED, "Peripheral not supported");
    fault |= checkFlag(flags, zml::ascii::warning_flags::PERIPHERAL_INACTIVE, "Peripheral inactive");
    fault |= checkFlag(flags, zml::ascii::warning_flags::HARDWARE_EMERGENCY_STOP, "Hardware emergency stop");
    fault |= checkFlag(flags, zml::ascii::warning_flags::OVERVOLTAGE_OR_UNDERVOLTAGE, "Overvoltage or undervoltage");
    fault |= checkFlag(flags, zml::ascii::warning_flags::DRIVER_DISABLED_NO_FAULT, "Driver disabled, no fault");
    fault |= checkFlag(flags, zml::ascii::warning_flags::CURRENT_INRUSH_ERROR, "Current inrush error");
    fault |= checkFlag(flags, zml::ascii::warning_flags::MOTOR_TEMPERATURE_ERROR, "Motor temperature error");
    fault |= checkFlag(flags, zml::ascii::warning_flags::DRIVER_DISABLED, "Driver disabled");
    fault |= checkFlag(flags, zml::ascii::warning_flags::ENCODER_ERROR, "Encoder error");
    fault |= checkFlag(flags, zml::ascii::warning_flags::INDEX_ERROR, "Index error");
    fault |= checkFlag(flags, zml::ascii::warning_flags::ANALOG_ENCODER_SYNC_ERROR, "Analog encoder sync error");
    fault |= checkFlag(flags, zml::ascii::warning_flags::OVERDRIVE_LIMIT_EXCEEDED, "Overdrive limit exceeded");
    fault |= checkFlag(flags, zml::ascii::warning_flags::STALLED_AND_STOPPED, "Stalled and stopped",
        [this]() { pC_->setIntegerParam(pC_->motorStatusFollowingError_, 1); });
    fault |= checkFlag(flags, zml::ascii::warning_flags::STREAM_BOUNDS_ERROR, "Stream bounds error");
    fault |= checkFlag(flags, zml::ascii::warning_flags::INTERPOLATED_PATH_DEVIATION, "Interpolated path deviation",
        [this]() { pC_->setIntegerParam(pC_->motorStatusFollowingError_, 1); });
    fault |= checkFlag(flags, zml::ascii::warning_flags::LIMIT_ERROR, "Limit error",
        [this]() { pC_->setIntegerParam(pC_->motorStatusLowLimit_, 1); pC_->setIntegerParam(pC_->motorStatusHighLimit_, 1); });
    fault |= checkFlag(flags, zml::ascii::warning_flags::EXCESSIVE_TWIST, "Excessive twist");
    fault |= checkFlag(flags, zml::ascii::warning_flags::UNEXPECTED_LIMIT_TRIGGER, "Unexpected limit trigger");
    return fault;
}

bool zaberAxis::checkFlag(std::unordered_set<std::string> flags, const std::string &flag,
    const std::string &message, std::function<void()> action) {
    
    if(flags.find(flag) == flags.end()) {
        return false;
    }

    if(flag[0] == 'F') {
        asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR,
            "Zaber Motion Fault: %s - %s\n", flag.c_str(), message.c_str());
    } else {
        asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR,
            "Zaber Motion Warning: %s - %s\n", flag.c_str(), message.c_str());
    }
    action();
    return true;
}

#include "zaberController.h"

#include <cmath>
#include <functional>

#include <epicsExport.h>
#include <epicsThread.h>
#include <epicsStdio.h>
#include <iocsh.h>

#include <zaber/motion/dto/ascii/device_identity.h>
#include <zaber/motion/ascii/axis_settings.h>
#include <zaber/motion/dto/ascii/measurement_sequence.h>
#include <zaber/motion/dto/ascii/digital_output_action.h>
#include <zaber/motion/ascii/pvt_sequence.h>
#include <zaber/motion/ascii/pvt_io.h>
#include <zaber/motion/ascii/pvt.h>
#include <zaber/motion/ascii/pvt_buffer.h>
#include <zaber/motion/dto/ascii/pvt_sequence_data.h>
#include <zaber/motion/units.h>

#include "zaberConnectionManager.h"
#include "zaberUtils.h"

static constexpr int  PVT_BUFFER_ID        = 1;
static constexpr int  PVT_SEQUENCE_ID      = 1;
static constexpr int  PULSE_OUTPUT_CHANNEL = 1;
static constexpr char POSITION_SETTING[]   = "pos";

namespace ze = zaber::epics;
namespace zml = zaber::motion;

/**
 * Creates a new zaberController object.
 * \param[in] portName          The name of the asyn port that will be created for this driver
 * \param[in] numAxes           The number of axes that this controller supports
 * \param[in] movingPollPeriod  The time between polls when any axis is moving
 * \param[in] idlePollPeriod    The time between polls when no axis is moving
 * \param[in] devicePort        The tcp or serial port that the zaber device is connected to
 * \param[in] deviceNumber      The number of the device on the port (1-indexed)
 */
zaberController::zaberController(const char *portName, int numAxes, double movingPollPeriod, double idlePollPeriod, const char *devicePort, int deviceNumber) :
        asynMotorController(portName, numAxes, 1, 0, 0, ASYN_CANBLOCK | ASYN_MULTIDEVICE, 1 /* autoconnect */, 0, 0) {
    createParam(ZaberPulseWidthMsString, asynParamFloat64, &zaberPulseWidthMs_);
    try {
        connection_ = ze::zaberConnectionManager::singleton().tryGetConnection(devicePort);
        device_ = connection_->getDevice(deviceNumber);
        device_.identify();
    } catch(const std::exception &e) {
        fprintf(stderr, "zaberController: Initialization failed\n\t%s\n", e.what());
    }
    for(int i = 0; i < numAxes; i++) {
        new zaberAxis(this, i);
    }
    startPoller(static_cast<double>(movingPollPeriod) / 1000.0, static_cast<double>(idlePollPeriod) / 1000.0, 2);
}

void zaberController::report(FILE *fp, int level) {
    std::function<asynStatus()> action = [this, fp]() {
        fprintf(fp, "Zaber Motion Controller: %s\n", this->portName);
        fprintf(fp, "  Zaber Device Info: %s\n", device_.toString().c_str());
        fprintf(fp, "  EPICS Num Axes: %d\n", numAxes_);
        return asynSuccess;
    };
    ze::handleException(this->pasynUserSelf, action);
    asynMotorController::report(fp, level);
 }

zml::ascii::Axis zaberController::getDeviceAxis(int axisNo) { return device_.getAxis(axisNo + 1); }

zaberAxis *zaberController::getAxis(asynUser *pasynUser) {
    return static_cast<zaberAxis *>(asynMotorController::getAxis(pasynUser));
}

zaberAxis *zaberController::getAxis(int axisNo) {
    return static_cast<zaberAxis*>(asynMotorController::getAxis(axisNo));
}

asynStatus zaberController::initializeProfile(size_t maxPoints) {
    return asynMotorController::initializeProfile(maxPoints);
}

asynStatus zaberController::buildProfile() {
    setStringParam(profileBuildMessage_, "Starting");
    setIntegerParam(profileBuildState_, PROFILE_BUILD_BUSY);
    setIntegerParam(profileBuildStatus_, PROFILE_STATUS_UNDEFINED);
    callParamCallbacks();

    asynMotorController::buildProfile();

    epicsInt32 moveMode;
    getIntegerParam(profileMoveMode_, &moveMode);
    if (moveMode == PROFILE_MOVE_MODE_RELATIVE) {
        const char *msg = "Relative move mode is not supported";
        setStringParam(profileBuildMessage_, msg);
        setIntegerParam(profileBuildStatus_, PROFILE_STATUS_FAILURE);
        setIntegerParam(profileBuildState_, PROFILE_BUILD_DONE);
        setIntegerParam(profileBuild_, 0);
        callParamCallbacks();
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "zaberController::buildProfile: %s\n", msg);
        return asynError;
    }

    epicsInt32 numPoints, numPulses, startPulses, endPulses;
    getIntegerParam(profileNumPoints_,  &numPoints);
    getIntegerParam(profileNumPulses_,  &numPulses);
    getIntegerParam(profileStartPulses_, &startPulses);
    getIntegerParam(profileEndPulses_,   &endPulses);

    if (numPulses > 0) {
        if (startPulses < 1 || endPulses > numPoints || numPulses != endPulses - startPulses + 1) {
            const char *msg =
                "Invalid pulse configuration: numPulses must equal endPulses - startPulses + 1, "
                "with 1 <= startPulses and endPulses <= numPoints";
            setStringParam(profileBuildMessage_, msg);
            setIntegerParam(profileBuildStatus_, PROFILE_STATUS_FAILURE);
            setIntegerParam(profileBuildState_, PROFILE_BUILD_DONE);
            setIntegerParam(profileBuild_, 0);
            callParamCallbacks();
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "zaberController::buildProfile: %s\n", msg);
            return asynError;
        }
    }

    double pulseWidthMs;
    getDoubleParam(zaberPulseWidthMs_, &pulseWidthMs);

    std::function<asynStatus()> action = [this, numPoints, numPulses, startPulses, endPulses, pulseWidthMs]() {
        std::vector<int> usedAxes;
        std::vector<zml::ascii::MeasurementSequence> positions;

        for (int i = 0; i < numAxes_; i++) {
            epicsInt32 used;
            getIntegerParam(i, profileUseAxis_, &used);
            if (used) {
                usedAxes.push_back(i + 1);
                const zaberAxis *axis = getAxis(i);
                if (axis == nullptr) {
                    throw std::runtime_error("Attempted to access invalid axis at index: " + std::to_string(i));
                }
                positions.push_back(zml::ascii::MeasurementSequence(std::vector<double>(numPoints), axis->lengthUnit_));
            }
        }

        zml::ascii::MeasurementSequence times(std::vector<double>(numPoints), zml::Units::TIME_SECONDS);
        for (int i = 0; i < numPoints; i++) {
            times.values[i] = profileTimes_[i];
            for (int j = 0; j < static_cast<int>(usedAxes.size()); j++) {
                positions[j].values[i] = getAxis(usedAxes[j] - 1)->profilePositions_[i];
            }
        }

        zml::ascii::PvtSequenceData data = zml::ascii::PvtSequence::generateVelocities(
            positions, times, {}, /*timesRelative=*/true);

        zml::ascii::Pvt pvt = device_.getPvt();
        zml::ascii::PvtSequence sequence = pvt.getSequence(PVT_SEQUENCE_ID);
        sequence.disable();
        zml::ascii::PvtBuffer buffer = pvt.getBuffer(PVT_BUFFER_ID);
        buffer.erase();
        sequence.setupStore(buffer, usedAxes);
        zml::ascii::PvtIo io = sequence.getIo();

        for (int i = 0; i < numPoints; i++) {
            std::vector<zml::Measurement> pointPositions;
            std::vector<std::optional<zml::Measurement>> pointVelocities;
            for (int j = 0; j < static_cast<int>(usedAxes.size()); j++) {
                pointPositions.push_back({data.positions[j].values[i], data.positions[j].unit});
                pointVelocities.push_back(zml::Measurement{data.velocities[j].values[i], data.velocities[j].unit});
            }
            zml::Measurement pointTime{data.times.values[i], data.times.unit};

            // Queue pulse output before the point it fires at (requires FW 7.37+).
            // IO actions are interleaved with PVT points in FIFO order.
            if (numPulses > 0 && i >= startPulses - 1 && i <= endPulses - 1) {
                io.setDigitalOutputSchedule(
                    PULSE_OUTPUT_CHANNEL,
                    zml::ascii::DigitalOutputAction::ON,
                    zml::ascii::DigitalOutputAction::OFF,
                    pulseWidthMs,
                    zml::Units::TIME_MILLISECONDS);
            }

            sequence.point(pointPositions, pointVelocities, pointTime);
        }

        sequence.disable();

        setStringParam(profileBuildMessage_, "Profile built successfully");
        setIntegerParam(profileBuildStatus_, PROFILE_STATUS_SUCCESS);
        setIntegerParam(profileBuildState_, PROFILE_BUILD_DONE);
        setIntegerParam(profileBuild_, 0);
        callParamCallbacks();
        return asynSuccess;
    };

    asynStatus status = ze::handleException(this->pasynUserSelf, action);
    if (status != asynSuccess) {
        const char *msg = "Profile build failed";
        setStringParam(profileBuildMessage_, msg);
        setIntegerParam(profileBuildStatus_, PROFILE_STATUS_FAILURE);
        setIntegerParam(profileBuildState_, PROFILE_BUILD_DONE);
        setIntegerParam(profileBuild_, 0);
        callParamCallbacks();
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "zaberController::buildProfile: %s\n", msg);
    }
    return status;
}

asynStatus zaberController::executeProfile() {
    std::vector<int> usedAxes;
    for (int i = 0; i < numAxes_; i++) {
        epicsInt32 used;
        getIntegerParam(i, profileUseAxis_, &used);
        if (used) {
            usedAxes.push_back(i + 1);
        }
    }

    double acceleration;
    getDoubleParam(profileAcceleration_, &acceleration);

    std::function<asynStatus()> action = [this, usedAxes, acceleration]() {
        epicsInt32 moveMode;
        getIntegerParam(profileMoveMode_, &moveMode);

        if (moveMode == PROFILE_MOVE_MODE_ABSOLUTE) {
            for (int axisNum : usedAxes) {
                zaberAxis *axis = getAxis(axisNum - 1);
                if (axis == nullptr) {
                    throw std::runtime_error("Attempted to access invalid axis at index: " + std::to_string(axisNum - 1));
                }
                axis->axis_.moveAbsolute(
                    axis->profilePositions_[0],
                    axis->lengthUnit_,
                    /*waitUntilIdle=*/true,
                    /*velocity=*/0,
                    zml::Units::NATIVE,
                    acceleration,
                    axis->accelUnit_);
            }
        }

        zml::ascii::Pvt pvt = device_.getPvt();
        zml::ascii::PvtSequence sequence = pvt.getSequence(PVT_SEQUENCE_ID);
        zml::ascii::PvtBuffer buffer = pvt.getBuffer(PVT_BUFFER_ID);
        sequence.disable();
        sequence.setupLive(usedAxes);
        sequence.call(buffer);
        sequence.waitUntilIdle();

        epicsInt32 numPulses;
        getIntegerParam(profileNumPulses_, &numPulses);
        setIntegerParam(profileActualPulses_, numPulses);
        setIntegerParam(profileExecute_, 0);
        callParamCallbacks();

        return asynSuccess;
    };

    return ze::handleException(this->pasynUserSelf, action);
}

asynStatus zaberController::abortProfile() {
    std::function<asynStatus()> action = [this]() {
        zml::ascii::Pvt pvt = device_.getPvt();
        pvt.getSequence(PVT_SEQUENCE_ID).disable();
        return asynSuccess;
    };
    return ze::handleException(this->pasynUserSelf, action);
}

asynStatus zaberController::readbackProfile() {
    setIntegerParam(profileReadbackState_,  PROFILE_READBACK_BUSY);
    setIntegerParam(profileReadbackStatus_, PROFILE_STATUS_UNDEFINED);
    callParamCallbacks();

    epicsInt32 numPulses, startPulses, endPulses;
    getIntegerParam(profileNumPulses_,   &numPulses);
    getIntegerParam(profileStartPulses_, &startPulses);
    getIntegerParam(profileEndPulses_,   &endPulses);

    std::function<asynStatus()> action = [this, numPulses, startPulses, endPulses]() {
        for (int i = 0; i < numAxes_; i++) {
            epicsInt32 used;
            getIntegerParam(i, profileUseAxis_, &used);
            if (!used) continue;

            zaberAxis *axis = getAxis(i);
            if (axis == nullptr) {
                throw std::runtime_error("Attempted to access invalid axis at index: " + std::to_string(i));
            }

            zml::ascii::AxisSettings settings = axis->axis_.getSettings();
            int pulseIndex = 0;
            for (int pt = startPulses - 1; pt <= endPulses - 1; pt++, pulseIndex++) {
                double requested = axis->profilePositions_[pt];
                // Round-trip through native units (microsteps) to reflect actual quantization.
                // profileFollowingErrors_ captures quantization error, not servo following error.
                double native  = std::round(settings.convertToNativeUnits(POSITION_SETTING, requested, axis->lengthUnit_));
                double rounded = settings.convertFromNativeUnits(POSITION_SETTING, native, axis->lengthUnit_);
                axis->profileReadbacks_[pulseIndex]       = rounded;
                axis->profileFollowingErrors_[pulseIndex] = rounded - requested;
            }
        }

        setIntegerParam(profileNumReadbacks_,  numPulses);
        setIntegerParam(profileActualPulses_,  numPulses);

        for (int i = 0; i < numAxes_; i++) {
            epicsInt32 used;
            getIntegerParam(i, profileUseAxis_, &used);
            if (!used) continue;
            getAxis(i)->readbackProfile();
        }

        setStringParam(profileReadbackMessage_,  "Readback complete");
        setIntegerParam(profileReadbackStatus_,  PROFILE_STATUS_SUCCESS);
        setIntegerParam(profileReadbackState_,   PROFILE_READBACK_DONE);
        setIntegerParam(profileReadback_, 0);
        callParamCallbacks();
        return asynSuccess;
    };

    asynStatus status = ze::handleException(this->pasynUserSelf, action);
    if (status != asynSuccess) {
        const char *msg = "Readback failed";
        setStringParam(profileReadbackMessage_,  msg);
        setIntegerParam(profileReadbackStatus_,  PROFILE_STATUS_FAILURE);
        setIntegerParam(profileReadbackState_,   PROFILE_READBACK_DONE);
        setIntegerParam(profileReadback_, 0);
        callParamCallbacks();
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "zaberController::readbackProfile: %s\n", msg);
    }
    return status;
}

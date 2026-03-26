#include "zaberController.h"

#include <cmath>
#include <functional>
#include <iostream>

#include <epicsEvent.h>
#include <epicsExport.h>
#include <epicsThread.h>
#include <epicsStdio.h>
#include <iocsh.h>

#include <zaber/motion/dto/unit_conversion_descriptor.h>
#include <zaber/motion/dto/ascii/device_identity.h>
#include <zaber/motion/dto/ascii/digital_output_action.h>
#include <zaber/motion/dto/ascii/pvt_partial_point.h>
#include <zaber/motion/dto/ascii/pvt_partial_sequence_item.h>
#include <zaber/motion/dto/ascii/pvt_sequence_item.h>
#include <zaber/motion/dto/ascii/pvt_set_digital_output_action.h>
#include <zaber/motion/ascii/axis_settings.h>
#include <zaber/motion/ascii/pvt_sequence.h>
#include <zaber/motion/ascii/pvt.h>
#include <zaber/motion/ascii/pvt_buffer.h>
#include <zaber/motion/units.h>
#include <zaber/motion/unit_table.h>

#include "zaberConnectionManager.h"
#include "zaberUtils.h"

static constexpr int  PVT_BUFFER_ID        = 1;
static constexpr int  PVT_SEQUENCE_ID      = 1;
static constexpr int  PULSE_OUTPUT_CHANNEL = 1;

namespace ze = zaber::epics;
namespace zml = zaber::motion;

static void zaberProfileThreadC(void *pPvt) {
    static_cast<zaberController *>(pPvt)->profileThread();
}

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

    profileExecuteEvent_ = epicsEventMustCreate(epicsEventEmpty);
    epicsThreadCreate("ZaberProfile",
                      epicsThreadPriorityLow,
                      epicsThreadGetStackSize(epicsThreadStackMedium),
                      (EPICSTHREADFUNC)zaberProfileThreadC, (void *)this);
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
        if (startPulses < 2 || endPulses > numPoints || numPulses != endPulses - startPulses + 1) {
            const char *msg =
                "Invalid pulse configuration: numPulses must equal endPulses - startPulses + 1, "
                "with 2 <= startPulses (pulses cannot be triggered at the profile's start position) "
                "and endPulses <= numPoints";
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
        for (int i = 0; i < numAxes_; i++) {
            epicsInt32 used;
            getIntegerParam(i, profileUseAxis_, &used);
            if (used) {
                if (getAxis(i) == nullptr) {
                    throw std::runtime_error("Attempted to access invalid axis at index: " + std::to_string(i));
                }
                usedAxes.push_back(i + 1);
            }
        }

        std::vector<zml::ascii::PvtPartialSequenceItem> partialItems;
        partialItems.reserve(numPoints + numPulses);
        for (int i = 0; i < numPoints; i++) {
            if (numPulses > 0 && i >= startPulses - 1 && i <= endPulses - 1) {
                partialItems.push_back(zml::ascii::PvtSetDigitalOutputAction(
                    PULSE_OUTPUT_CHANNEL,
                    zml::ascii::DigitalOutputAction::ON,
                    zml::Measurement{pulseWidthMs, zml::Units::TIME_MILLISECONDS},
                    zml::ascii::DigitalOutputAction::OFF));
            }
            std::vector<std::optional<zml::Measurement>> pointPositions;
            std::vector<std::optional<zml::Measurement>> pointVelocities;
            for (int axisNum : usedAxes) {
                const zaberAxis *axis = getAxis(axisNum - 1);
                pointPositions.push_back(zml::Measurement{axis->profilePositions_[i], axis->lengthUnit_});
            }
            // Point 0 is the starting position.
            // This driver will move the device to this starting position when the profile is executed.
            partialItems.push_back(zml::ascii::PvtPartialPoint(
                pointPositions,
                pointVelocities,
                zml::Measurement{i == 0 ? 0.0 : profileTimes_[i], zml::Units::TIME_SECONDS},
                /*relative=*/false));
        }

        std::vector<zml::ascii::PvtSequenceItem> sequenceData =
            zml::ascii::PvtSequence::generateVelocities(partialItems);

        for (const zml::ascii::PvtSequenceItem& item : sequenceData) {
            std::cout << zml::ascii::PvtSequenceItem_toString(item) << std::endl;
        }

        zml::ascii::Pvt pvt = device_.getPvt();
        zml::ascii::PvtSequence sequence = pvt.getSequence(PVT_SEQUENCE_ID);
        sequence.disable();
        zml::ascii::PvtBuffer buffer = pvt.getBuffer(PVT_BUFFER_ID);
        buffer.erase();
        sequence.setupStore(buffer, usedAxes);
        sequence.submitSequenceData({sequenceData.begin() + 1, sequenceData.end()});
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
    epicsEventSignal(profileExecuteEvent_);
    return asynSuccess;
}

void zaberController::profileThread() {
    while (true) {
        epicsEventWait(profileExecuteEvent_);
        runProfile();
    }
}

void zaberController::runProfile() {
    lock();
    std::vector<int> usedAxes;
    for (int i = 0; i < numAxes_; i++) {
        epicsInt32 used;
        getIntegerParam(i, profileUseAxis_, &used);
        if (used) {
            usedAxes.push_back(i + 1);
        }
    }
    epicsInt32 moveMode;
    getIntegerParam(profileMoveMode_, &moveMode);
    setIntegerParam(profileExecuteState_, PROFILE_EXECUTE_MOVE_START);
    setIntegerParam(profileExecuteStatus_, PROFILE_STATUS_UNDEFINED);
    callParamCallbacks();
    unlock();

    std::function<asynStatus()> action = [this, usedAxes, moveMode]() {
        wakeupPoller();

        if (moveMode == PROFILE_MOVE_MODE_ABSOLUTE) {
            for (int axisNum : usedAxes) {
                // Move to start position using device's current velocity/acceleration settings.
                zaberAxis *axis = getAxis(axisNum - 1);
                axis->axis_.moveAbsolute(axis->profilePositions_[0], axis->lengthUnit_, /*waitUntilIdle=*/true);
            }
        }

        lock();
        setIntegerParam(profileExecuteState_, PROFILE_EXECUTE_EXECUTING);
        callParamCallbacks();
        unlock();

        zml::ascii::Pvt pvt = device_.getPvt();
        zml::ascii::PvtSequence sequence = pvt.getSequence(PVT_SEQUENCE_ID);
        zml::ascii::PvtBuffer buffer = pvt.getBuffer(PVT_BUFFER_ID);
        sequence.disable();
        sequence.setupLive(usedAxes);
        sequence.call(buffer);
        sequence.waitUntilIdle();
        sequence.disable();

        lock();
        epicsInt32 numPulses;
        getIntegerParam(profileNumPulses_, &numPulses);
        setIntegerParam(profileActualPulses_, numPulses);
        setStringParam(profileExecuteMessage_, "Execute complete");
        setIntegerParam(profileExecuteStatus_, PROFILE_STATUS_SUCCESS);
        setIntegerParam(profileExecuteState_, PROFILE_EXECUTE_DONE);
        setIntegerParam(profileExecute_, 0);
        callParamCallbacks();
        unlock();

        return asynSuccess;
    };

    asynStatus status = ze::handleException(this->pasynUserSelf, action);
    if (status != asynSuccess) {
        const char *msg = "Execute failed";
        lock();
        setStringParam(profileExecuteMessage_, msg);
        setIntegerParam(profileExecuteStatus_, PROFILE_STATUS_FAILURE);
        setIntegerParam(profileExecuteState_, PROFILE_EXECUTE_DONE);
        setIntegerParam(profileExecute_, 0);
        callParamCallbacks();
        unlock();
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "zaberController::runProfile: %s\n", msg);
    }
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

            std::vector<double> positions;
            positions.reserve(numPulses);
            for (int pointIndex = startPulses - 1; pointIndex <= endPulses - 1; pointIndex++) {
                positions.push_back(axis->profilePositions_[pointIndex]);
            }

            // Round-trip positions through native units (microsteps) to reflect actual positions.
            // FW guarantees that PVT IO actions are triggered at exactly the position of
            // the preceding point in the sequence, so these deltas affect the physical error.
            zml::UnitConversionDescriptor descriptor = axis->axis_.getSettings().getUnitConversionDescriptor("pos");
            std::vector<double> native = zml::UnitTable::convertToNativeUnitsBatch(descriptor, positions, axis->lengthUnit_);
            for (auto& v : native) v = std::round(v);
            std::vector<double> quantized = zml::UnitTable::convertFromNativeUnitsBatch(descriptor, native, axis->lengthUnit_);

            int pulseIndex = 0;
            for (int pointIndex = startPulses - 1; pointIndex <= endPulses - 1; pointIndex++, pulseIndex++) {
                axis->profileReadbacks_[pulseIndex] = quantized[pulseIndex];
                axis->profileFollowingErrors_[pulseIndex] = quantized[pulseIndex] - axis->profilePositions_[pointIndex];
            }
        }

        setIntegerParam(profileNumReadbacks_, numPulses);
        setIntegerParam(profileActualPulses_, numPulses);

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

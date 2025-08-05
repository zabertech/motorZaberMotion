#include "zaberController.h"

#include <functional>

#include <epicsExport.h>
#include <epicsThread.h>
#include <epicsStdio.h>
#include <iocsh.h>

#include <zaber/motion/dto/ascii/device_identity.h>
#include <zaber/motion/dto/ascii/measurement_sequence.h>
#include <zaber/motion/ascii/pvt_sequence.h>
#include <zaber/motion/ascii/pvt.h>
#include <zaber/motion/ascii/pvt_buffer.h>
#include <zaber/motion/dto/ascii/pvt_sequence_data.h>
#include <zaber/motion/units.h>

#include "zaberConnectionManager.h"
#include "zaberUtils.h"

#define PROFILE_MOVE_ERR_STR "Zaber Motion Error: Profile moves not implemented for zaberController\n"

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
        asynMotorController(portName, numAxes, 0, 0, 0, ASYN_CANBLOCK | ASYN_MULTIDEVICE, 1 /* autoconnect */, 0, 0) {
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
    int numPoints;

    setStringParam(profileBuildMessage_, "Starting");
    setIntegerParam(profileBuildState_, PROFILE_BUILD_BUSY);
    setIntegerParam(profileBuildStatus_, PROFILE_STATUS_UNDEFINED);
    callParamCallbacks();

    asynMotorController::buildProfile();

    getIntegerParam(profileNumPoints_, &numPoints);

    std::function<asynStatus()> action = [this, numPoints]() {
        std::vector<int> usedAxes;
        std::vector<zml::ascii::MeasurementSequence> positions;

        for (int i = 0; i < numAxes_; i++) {
            epicsInt32 used;
            getIntegerParam(i, profileUseAxis_, &used);
            if (used) {
                usedAxes.push_back(i+1);
                const zaberAxis* axis = getAxis(i);
                if (axis == NULL) {
                    throw new std::runtime_error("Attempted to access invalid axis at index: " + std::to_string(i));
                }
                positions.push_back(zml::ascii::MeasurementSequence(std::vector<double>(numPoints), getAxis(i)->lengthUnit_));
            }
        }

        zml::ascii::MeasurementSequence times(std::vector<double>(numPoints), zml::Units::TIME_SECONDS);
        for (int i = 0; i < numPoints; i++) {
            times.values[i] = profileTimes_[i];
            for (int j = 0; j < static_cast<int>(usedAxes.size()); j++) {
                positions[j].values[i] = getAxis(usedAxes[j]-1)->profilePositions_[i];
            }
        }

        zml::ascii::PvtSequenceData data = zml::ascii::PvtSequence::generateVelocities(positions, times);
        zml::ascii::Pvt pvt = device_.getPvt();
        zml::ascii::PvtSequence sequence = pvt.getSequence(1);
        sequence.disable();
        zml::ascii::PvtBuffer buffer = pvt.getBuffer(1);
        buffer.erase();
        sequence.setupStore(buffer, 1);
        sequence.points(data.positions, data.velocities, data.times);
        sequence.disable();

        return asynSuccess;
    };

    return zaber::epics::handleException(this->pasynUserSelf, action);
}

asynStatus zaberController::executeProfile() {
    std::vector<int> usedAxes;
    for (int i = 0; i < numAxes_; i++) {
        epicsInt32 used;
        getIntegerParam(i, profileUseAxis_, &used);
        if (used) {
            usedAxes.push_back(i+1);
        }
    }

    zml::ascii::Pvt pvt = device_.getPvt();
    zml::ascii::PvtSequence sequence = pvt.getSequence(1);
    zml::ascii::PvtBuffer buffer = pvt.getBuffer(1);
    sequence.disable();
    sequence.setupLive(1); // initializer list in these functions is really inconvenient -- TODO fix it
    sequence.call(buffer);

    // asynPrint(this->pasynUserSelf, ASYN_TRACE_WARNING, PROFILE_MOVE_ERR_STR);
    return asynSuccess;
}

asynStatus zaberController::abortProfile() {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_WARNING, PROFILE_MOVE_ERR_STR);
    return asynError;
}

asynStatus zaberController::readbackProfile() {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_WARNING, PROFILE_MOVE_ERR_STR);
    return asynError;
}

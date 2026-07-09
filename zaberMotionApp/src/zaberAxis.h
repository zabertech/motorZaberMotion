#ifndef ZABER_AXIS_H
#define ZABER_AXIS_H

#include <unordered_set>
#include <unordered_map>
#include <functional>

#include <asynMotorAxis.h>
#include <zaber/motion/ascii/axis.h>
#include <zaber/motion/units.h>

namespace zml = zaber::motion;

class zaberController;

class epicsShareClass zaberAxis : public asynMotorAxis {
    public:
    zaberAxis(zaberController *pC, int axisNo, double unitsPerStep);
    ~zaberAxis();
    void report(FILE *fp, int details) override;
    asynStatus move(double position, int relative, double minVelocity, double maxVelocity, double acceleration) override;
    asynStatus moveVelocity(double minVelocity, double maxVelocity, double acceleration) override;
    asynStatus home(double minVelocity, double maxVelocity, double acceleration, int forwards) override;
    asynStatus stop(double acceleration) override;
    asynStatus poll(bool *moving) override;
    asynStatus setPosition(double position) override;

    private:
    static const std::unordered_map<std::string, std::string> ZML_FAULT_TO_MESSAGE;
    zaberController *pC_;
    zml::ascii::Axis axis_;
    zml::Units positionUnit_;
    zml::Units velocityUnit_;
    zml::Units accelUnit_;
    // Size of one motor-record step in positionUnit_ (um/deg) -- the same step the
    // record's MRES describes in EGU. Defaults to 1.0 (1 um or 1 deg per step).
    double unitsPerStep_ = 1.0;

    asynStatus doAbsoluteMove(double position, double velocity, double acceleration);
    asynStatus doRelativeMove(double distance, double velocity, double acceleration);

    inline bool checkAllFlags(const std::unordered_set<std::string> &flags);

    friend class zaberController;
};

#endif // ZABER_AXIS_H

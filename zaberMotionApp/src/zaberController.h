#ifndef ZABER_BASE_CONTROLLER_H
#define ZABER_BASE_CONTROLLER_H

#include <asynMotorController.h>

#include <zaber/motion/ascii/axis.h>
#include <zaber/motion/ascii/connection.h>
#include <zaber/motion/ascii/device.h>

#include "zaberAxis.h"

class zaberAxis;

class epicsShareClass zaberController : public asynMotorController {
public:
    zaberController(const char *portName, int numAxes, const char *serialPort, int deviceId, int priority, int stackSize, int extMotorParams);
    void report(FILE *fp, int level) override;
    void zaberPoller();
    static void callPoller(void *);
    static void callShutdown(void *ptr) { ((zaberController *)ptr)->shutdown(); };
    void shutdown();

    /* These are the methods that are new to this class */
    zaberAxis *getAxis(asynUser *pasynUser);
    zaberAxis *getAxis(int axisNo);
    // asynStatus wakeupPoller();
    // asynStatus poll();
    // asynStatus setDeferredMoves(bool defer);
    void asynMotorPoller(); // This should be private but is called from C function

    /* Functions to deal with moveToHome.*/
    // asynStatus startMoveToHomeThread();
    // void asynMotorMoveToHome(); not typically overridden

    // TODO: Decide whether or not to implement profile moves
    // you could set up PVT in this way, but the system seems to not allow users
    // to add velocity or time information
    asynStatus initializeProfile(size_t maxPoints) override;
    asynStatus buildProfile() override;
    asynStatus executeProfile() override;
    asynStatus abortProfile() override;
    asynStatus readbackProfile() override;

    // asynStatus setMovingPollPeriod(double movingPollPeriod);
    // asynStatus setIdlePollPeriod(double idlePollPeriod);

private:
    friend zaberAxis;

    static int zaberTotalControllerNumber;
    zaber::motion::ascii::Connection connection_;
    zaber::motion::ascii::Device device_;

    zaber::motion::ascii::Axis getDeviceAxis(int axisNo);
};

#endif
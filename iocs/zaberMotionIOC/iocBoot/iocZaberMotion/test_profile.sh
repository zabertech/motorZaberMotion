#!/bin/bash
# Test script for Zaber PVT profile moves.
# Run after the IOC is started. Requires caput/caget on PATH.
#
# Profile: 5 points, 1 second per segment, 3 sync pulses on points 2-4.
# Positions are in user units (mm), matching MRES=0.001 and EGU=mm.

set -e

P="IOC:MyProfile:"
AXIS="M1"

wait_for_done() {
    local pv="$1"
    local timeout=30
    local elapsed=0
    echo "  Waiting on $pv..."
    while [ "$(caget -t "$pv")" != "0" ]; do
        sleep 0.5
        elapsed=$((elapsed + 1))
        if [ "$elapsed" -ge "$((timeout * 2))" ]; then
            echo "ERROR: Timed out waiting for $pv"
            exit 1
        fi
    done
}

echo "=== Zaber Profile Move Test ==="

echo "--- Configuring profile parameters ---"
caput "${P}NumPoints"   5
caput "${P}TimeMode"    0            # Fixed time mode
caput "${P}FixedTime"   1.0          # 1 second per segment
caput "${P}MoveMode"    0            # Absolute
caput "${P}NumPulses"   3
caput "${P}StartPulses" 2
caput "${P}EndPulses"   4
caput "${P}PulseWidthMs" 10.0        # 10 ms pulse width (Zaber-specific, FW 7.37+)

echo "--- Configuring axis ---"
caput "${P}${AXIS}UseAxis" 1

echo "--- Writing position waveform (mm) ---"
# 5 positions: move from 10 mm to 50 mm in 10 mm steps
caput -a "${P}${AXIS}Positions" 5   10.0 20.0 30.0 40.0 50.0

echo "--- Building profile ---"
caput "${P}Build" 1
wait_for_done "${P}Build"
BUILD_STATUS=$(caget -t "${P}BuildStatus")
BUILD_MSG=$(caget -t "${P}BuildMessage")
echo "  BuildStatus: $BUILD_STATUS"
echo "  BuildMessage: $BUILD_MSG"
if [ "$BUILD_STATUS" != "1" ]; then
    echo "ERROR: Build did not succeed"
    exit 1
fi

echo "--- Executing profile ---"
caput "${P}Execute" 1
wait_for_done "${P}Execute"
EXEC_STATUS=$(caget -t "${P}ExecuteStatus")
echo "  ExecuteStatus: $EXEC_STATUS"
ACTUAL_PULSES=$(caget -t "${P}NumActualPulses")
echo "  Actual pulses: $ACTUAL_PULSES"
if [ "$EXEC_STATUS" != "1" ]; then
    echo "ERROR: Execute did not succeed"
    exit 1
fi

echo "--- Reading back positions ---"
caput "${P}Readback" 1
wait_for_done "${P}Readback"
echo "  Readbacks:"
caget "${P}${AXIS}Readbacks"
echo "  Following errors:"
caget "${P}${AXIS}FollowingErrors"

echo "=== Test complete ==="

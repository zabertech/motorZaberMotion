#!/bin/bash
# Test script for Zaber PVT profile move.
# Run after the IOC is started with st.cmd.zy-stage script.
# Requires caput/caget on PATH.
#
# Profile: 50 points, 1.0 second per segment, 31 sync pulses on points 10-40.
# Circle: center=(50,50)mm, radius=40mm. Both axes stay within 10-90mm.

set -e

P="IOC:MyProfile:"
AXIS1="M1"
AXIS2="M2"

wait_for_done() {
    local pv="$1"
    local timeout=180
    local elapsed=0
    echo "  Waiting on $pv..."
    while [ "$(caget -t -n "$pv")" != "0" ]; do
        sleep 0.5
        elapsed=$((elapsed + 1))
        if [ "$elapsed" -ge "$((timeout * 2))" ]; then
            echo "ERROR: Timed out waiting for $pv"
            exit 1
        fi
    done
}

# 50-point circle: center=(50,50)mm, radius=40mm, closes at i=49
X_POSITIONS="90.000 89.672 88.692 87.077 84.853 82.057 78.734 74.940 70.736 66.191 61.381 56.384 51.282 46.159 41.099 36.185 31.498 27.115 23.108 19.542 16.476 13.961 12.038 10.738 10.082 10.082 10.738 12.038 13.961 16.476 19.542 23.108 27.115 31.498 36.185 41.099 46.159 51.282 56.384 61.381 66.191 70.736 74.940 78.734 82.057 84.853 87.077 88.692 89.672 90.000"
Y_POSITIONS="50.000 55.115 60.146 65.011 69.629 73.924 77.827 81.273 84.206 86.577 88.347 89.487 89.979 89.815 88.997 87.539 85.464 82.807 79.611 75.929 71.821 67.355 62.604 57.646 52.563 47.437 42.354 37.396 32.645 28.179 24.071 20.389 17.193 14.536 12.461 11.003 10.185 10.021 10.513 11.653 13.423 15.794 18.727 22.173 26.076 30.371 34.989 39.854 44.885 50.000"

echo "=== Zaber Circular Profile Move Test ==="

echo "--- Configuring profile parameters ---"
caput "${P}NumPoints"    50
caput "${P}TimeMode"     0          # Fixed time mode
caput "${P}FixedTime"    0.2        # 0.25 second per segment
caput "${P}MoveMode"     0          # Absolute
caput "${P}NumPulses"    31
caput "${P}StartPulses"  10
caput "${P}EndPulses"    40
caput "${P}PulseWidthMs" 20.0       # 20ms pulse width

echo "--- Configuring axes ---"
caput "${P}${AXIS1}UseAxis" 1
caput "${P}${AXIS2}UseAxis" 1

echo "--- Writing position waveforms (mm) ---"
caput -a "${P}${AXIS1}Positions" 50 $X_POSITIONS
caput -a "${P}${AXIS2}Positions" 50 $Y_POSITIONS

echo "--- Building profile ---"
caput "${P}Build" 1
wait_for_done "${P}Build"
BUILD_STATUS=$(caget -t -n "${P}BuildStatus")
echo "  BuildStatus: $BUILD_STATUS"
if [ "$BUILD_STATUS" != "1" ]; then
    echo "  BuildMessage: $(caget -t -S "${P}BuildMessage")"
    echo "ERROR: Build did not succeed"
    exit 1
fi

echo "--- Executing profile ---"
caput "${P}Execute" 1
wait_for_done "${P}Execute"
EXEC_STATUS=$(caget -t -n "${P}ExecuteStatus")
echo "  ExecuteStatus: $EXEC_STATUS"
ACTUAL_PULSES=$(caget -t -n "${P}NumActualPulses")
echo "  Actual pulses: $ACTUAL_PULSES"
if [ "$EXEC_STATUS" != "1" ]; then
    echo "  ExecuteMessage: $(caget -t -S "${P}ExecuteMessage")"
    echo "ERROR: Execute did not succeed"
    exit 1
fi

echo "--- Reading back positions ---"
caput "${P}Readback" 1
wait_for_done "${P}Readback"
echo "  Axis 1 readbacks:"
caget "${P}${AXIS1}Readbacks"
echo "  Axis 1 following errors:"
caget "${P}${AXIS1}FollowingErrors"
echo "  Axis 2 readbacks:"
caget "${P}${AXIS2}Readbacks"
echo "  Axis 2 following errors:"
caget "${P}${AXIS2}FollowingErrors"

echo "=== Test complete ==="

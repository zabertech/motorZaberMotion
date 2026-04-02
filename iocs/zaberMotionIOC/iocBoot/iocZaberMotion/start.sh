#!/bin/bash
# Usage: ./start.sh [st.cmd file]
# Defaults to st.cmd.linear-stage if no argument is given.
# Requires EPICS_HOST_ARCH to be set in the environment.
#
# Example:
#   ./start.sh st.cmd.xy-stage

CMD=${1:-st.cmd.linear-stage}
exec ../../bin/${EPICS_HOST_ARCH}/zaberMotion "${CMD}"

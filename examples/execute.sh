#!/bin/bash -e

: ${PYTHON:=python3}
: ${ARGS:="-i 100 -f 36 -c 19"}

rm -rf DISABLED* ENABLED_WALL_CLOCK* \
    timemory-enabled-wall-clock-output

#----------------------------------------------------------#
#       Disabled
#----------------------------------------------------------#
unset CALI_CONFIG_PROFILE
export TIMEMORY_ENABLED=OFF
${PYTHON} ./execute.py -p DISABLED ${ARGS} $@

#----------------------------------------------------------#
#       Enabled
#----------------------------------------------------------#
export CALI_CONFIG_PROFILE=runtime-report
export TIMEMORY_ENABLED=ON
export TIMEMORY_OUTPUT_PATH=timemory-enabled-wall-clock-output
${PYTHON} ./execute.py -p ENABLED_WALL_CLOCK ${ARGS} $@

#!/bin/bash

: ${ARGS:="-i 100 -f 36 -c 19"}

rm -rf DISABLED* ENABLED* \
    timemory-enabled-output

export TAU_THROTTLE=0
export TAU_TRACE=0
export TAU_SUMMARY=0
export TAU_SET_NODE=0

#----------------------------------------------------------#
#       Disabled
#----------------------------------------------------------#
unset CALI_CONFIG_PROFILE
export TAU_PROFILE=0
export TIMEMORY_ENABLED=OFF
export TIMEMORY_OUTPUT_PATH=timemory-disabled-output

set -e
python ./execute.py -p DISABLED ${ARGS} $@
set +e

#----------------------------------------------------------#
#       Enabled
#----------------------------------------------------------#
export CALI_CONFIG_PROFILE=runtime-report
export TAU_PROFILE=1
export TIMEMORY_ENABLED=ON
export TIMEMORY_OUTPUT_PATH=timemory-enabled-output

set -e
python ./execute.py -p ENABLED ${ARGS} $@
set +e

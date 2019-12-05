#!/bin/bash

: ${ARGS:="-i 100 -f 36 -c 19"}

rm -rf DISABLED* ENABLED* \
    timemory-enabled-output

#----------------------------------------------------------#
#       Disabled
#----------------------------------------------------------#
unset CALI_CONFIG_PROFILE
export TIMEMORY_ENABLED=OFF
export TAU_PROFILE=0

set -e
python ./execute.py -p DISABLED ${ARGS} $@
set +e

#----------------------------------------------------------#
#       Enabled
#----------------------------------------------------------#
export CALI_CONFIG_PROFILE=runtime-report
export TIMEMORY_ENABLED=ON
export TAU_PROFILE=1
export TIMEMORY_OUTPUT_PATH=timemory-enabled-output

set -e
python ./execute.py -p ENABLED ${ARGS} $@
set +e

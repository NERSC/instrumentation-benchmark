#!/bin/bash -e

: ${ARGS:="-i 50 -f 40 -c 15"}

rm -rf DISABLED* ENABLED_WALL_CLOCK* \
    timemory-enabled-wall-clock-output

#----------------------------------------------------------#
#       Disabled
#----------------------------------------------------------#
unset CALI_CONFIG_PROFILE
export TIMEMORY_ENABLED=OFF
python ./execute.py -p DISABLED ${ARGS} $@

#----------------------------------------------------------#
#       Enabled
#----------------------------------------------------------#
export CALI_CONFIG_PROFILE=runtime-report
export TIMEMORY_ENABLED=ON
export TIMEMORY_OUTPUT_PATH=timemory-enabled-wall-clock-output
python ./execute.py -p ENABLED_WALL_CLOCK ${ARGS} $@

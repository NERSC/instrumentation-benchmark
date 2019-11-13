#!/bin/bash -e

: ${ARGS:="-i 100 -f 36 -c 19"}

rm -rf DISABLED* ENABLED* \
    timemory-enabled-output

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
export TIMEMORY_OUTPUT_PATH=timemory-enabled-output
python ./execute.py -p ENABLED ${ARGS} $@

#!/bin/bash -e

: ${PYTHON:=python3}
: ${ARGS:="-i 200 -f 36 -c 19 -e 100"}

rm -rf DISABLED* ENABLED* \
    timemory-enabled-output

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
export TIMEMORY_OUTPUT_PATH=timemory-enabled-output
${PYTHON} ./execute.py -p ENABLED ${ARGS} $@

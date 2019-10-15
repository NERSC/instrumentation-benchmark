#!/bin/bash -e

: ${ARGS:="-i 50 -l cxx"}

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
python ./execute.py -p ENABLED_WALL_CLOCK ${ARGS} $@

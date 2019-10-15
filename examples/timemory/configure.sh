#!/bin/bash -e

#-----------------------------------------------------------#
#
#     Configure project for timemory benchmarking
#
#-----------------------------------------------------------#

THIS_DIR=$(dirname ${BASH_SOURCE[0]})
INC_DIR=${THIS_DIR}/../../include/user
CMAKE_DIR=${THIS_DIR}/../../cmake

# echo "${THIS_DIR}"
# echo "${INC_DIR}"
# echo "${CMAKE_DIR}"

cp ${THIS_DIR}/*.h ${INC_DIR}/
cp ${THIS_DIR}/*.hpp ${INC_DIR}/
cp ${THIS_DIR}/*.cmake ${CMAKE_DIR}/

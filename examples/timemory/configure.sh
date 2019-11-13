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

for i in ${THIS_DIR}/*.h ${THIS_DIR}/*.hpp
do
    ln -sf ../../${i} ${INC_DIR}/$(basename ${i})
done

for i in ${THIS_DIR}/*.cmake
do
    ln -sf ../${i} ${CMAKE_DIR}/$(basename ${i})
done

# cp ${THIS_DIR}/*.h ${INC_DIR}/
# cp ${THIS_DIR}/*.hpp ${INC_DIR}/
# cp ${THIS_DIR}/*.cmake ${CMAKE_DIR}/

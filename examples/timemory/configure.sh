#!/bin/bash -e

#-----------------------------------------------------------#
#
#     Configure project for timemory benchmarking
#
#-----------------------------------------------------------#

SCRIPT_DIR=$(dirname ${BASH_SOURCE[0]})
THIS_DIR=$(python -c "import os; print(os.path.realpath('${SCRIPT_DIR}'))")
BASE_DIR=$(python -c "import os; print(os.path.realpath('${SCRIPT_DIR}/../..'))")

INC_DIR=${BASE_DIR}/include/user
CMAKE_DIR=${BASE_DIR}/cmake

remove-path()
{
    echo $2 | sed s,${1}/,,g
}

HEADERS=$(ls ${THIS_DIR}/*.h ${THIS_DIR}/*.hpp)
for i in ${HEADERS}
do
    fname=$(basename ${i})
    lpath="../../examples/timemory/${fname}"
    ln -sf ${lpath} ${INC_DIR}/${fname}
done

cd ../..

for i in $(ls ${THIS_DIR}/*.cmake)
do
    fname=$(basename ${i})
    lpath="../../cmake/${fname}"
    ln -sf ${lpath} ${CMAKE_DIR}/${fname}
done

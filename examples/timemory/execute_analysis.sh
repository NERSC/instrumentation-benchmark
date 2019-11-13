#!/bin/bash


export TIMEMORY_JSON_OUTPUT=ON

check()
{
    local RESULT=""
    for i in $@
    do
        local AVAIL=$(papi_avail | grep Yes | awk '{print $1}' | grep $i)
        if [ -n "${AVAIL}" ]; then RESULT="${RESULT}${AVAIL},"; fi
    done
    echo "${RESULT}"
}

run()
{
    local BASE_LABEL=$1
    for i in 36
    do
        LABEL="${BASE_LABEL}-${i}"
        export TIMEMORY_OUTPUT_PATH="timemory-${LABEL}-output"
        python3 ./execute.py ${ARGS} -p ${LABEL} -f ${i} -i 100 -c 19
        mv ${LABEL}.txt ${TIMEMORY_OUTPUT_PATH}/
        echo ""
    done
}

for level in 1 2 3;
do
    for mode in M R A W
    do
        COUNTERS=$(check PAPI_L${level}_DC${mode} PAPI_L${level}_IC${mode} PAPI_L${level}_TC${mode})
        if [ -n "${COUNTERS}" ]; then
            export TIMEMORY_PAPI_EVENTS="${COUNTERS}"
            run "L${level}-C${mode}"
        fi
    done
done

export TIMEMORY_PAPI_EVENTS="PAPI_BR_UCN, PAPI_BR_CN"
run "BRANCHING-COND"

export TIMEMORY_PAPI_EVENTS="PAPI_BR_TKN, PAPI_BR_NTK"
run "BRANCHING-TAKEN"

export TIMEMORY_PAPI_EVENTS="PAPI_BR_MSP, PAPI_BR_PRC"
run "BRANCHING-PREDICT"

export TIMEMORY_PAPI_EVENTS="PAPI_TOT_CYC, PAPI_RES_STL, PAPI_PRF_DM"
run "CYCLE"

export TIMEMORY_PAPI_EVENTS="PAPI_TOT_INS, PAPI_LST_INS, PAPI_BR_INS"
run "INST"

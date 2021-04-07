#!/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/../../
logfile=ibofos.log

execute_ibofos()
{
    if [ -f ${ROOT_DIR}/bin/ibofos ];
    then
        echo "Execute ibofos"
        nohup ${ROOT_DIR}/bin/ibofos &>> ${ROOT_DIR}/script/${logfile} &
    else
        echo "No executable ibofos file"
        exit -1
    fi
}

check_started()
{
    result=`${ROOT_DIR}/bin/cli request info --json | jq '.Response.info.state' 2>/dev/null`
    if [ -z ${result} ] || [ ${result} != '"OFFLINE"' ];
    then
        return 0
    else
        return 1
    fi
}

wait_started()
{
    check_started
    while [ $? -eq 0 ];
    do
        echo "Wait ibofos"
        sleep 0.5
        check_started
    done
}

execute_ibofos
wait_started

echo "ibofos is running in background...logfile=${logfile}"

#!/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/../../
logfile=pos.log
binary_name=poseidonos

execute_pos()
{
    if [ -f ${ROOT_DIR}/bin/$binary_name ];
    then
        echo "Execute poseidonOS"
        nohup ${ROOT_DIR}/bin/$binary_name &>> ${ROOT_DIR}/script/${logfile} &
    else
        echo "No executable poseidonOS file"
        exit -1
    fi
}

check_started()
{
    result=`${ROOT_DIR}/bin/cli system info --json | jq '.Response.info.version' 2>/dev/null`

    if [ -z ${result} ] || [ ${result} == '""' ];
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
        echo "Wait poseidonOS"
        sleep 0.5
        check_started
    done
}

execute_pos
wait_started

echo "PoseidonOS version =" ${result}
echo "poseidonOS is running in background...logfile=${logfile}"

#!/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/../../
logfile=pos.log
binary_name=poseidonos

setup_environment()
{
    ${ROOT_DIR}/script/setup_env.sh
    rm -rf /dev/shm/ibof_nvmf_trace*
}

execute_ibofos()
{
    if [ -f ${ROOT_DIR}/bin/$binary_name ];
    then
        echo "Execute poseidonos"
        nohup ${ROOT_DIR}/bin/$binary_name &>> ${ROOT_DIR}/script/${logfile} &
    else
        echo "No executable poseidonos file"
        exit -1
    fi
}

check_started()
{
    result=`${ROOT_DIR}/bin/cli system info --json | jq '.Response.data.version' 2>/dev/null`

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
        echo "Wait poseidonos"
        sleep 0.5
        check_started
    done
}

if [[ ! -z "$1" ]];then
    binary_name=$1
fi

setup_environment
execute_ibofos
wait_started

echo "poseidonos is running in background...logfile=${logfile}"

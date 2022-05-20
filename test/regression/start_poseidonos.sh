#!/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/../../
binary_name=poseidonos

execute_pos()
{
    if [ -f ${ROOT_DIR}/bin/$binary_name ];
    then
        echo "Execute poseidonOS"
        nohup ${ROOT_DIR}/bin/$binary_name > /dev/null 2>&1 &
    else
        echo "No executable poseidonOS file"
        exit -1
    fi
}

check_started()
{
    result=`${ROOT_DIR}/bin/poseidonos-cli system info --json-res | jq '.Response.info.version' 2>/dev/null`

    if [ -z ${result} ] || [ ${result} == '""' ];
    then
        return 0
    else
        return 1
    fi
}

wait_started()
{
    loop_count=0
    check_started
    while [ $? -eq 0 ];
    do
        let loop_count=loop_count+1
        echo "Wait poseidonos for $loop_count seconds"
        sleep 1
        if [ $loop_count -gt 300 ];then
            echo "Failed to execute poseidonos"
            exit -1
        fi
        check_started
    done
}

execute_pos
wait_started

echo "PoseidonOS version =" ${result}
echo "poseidonOS is running in background..."

#!/bin/bash

pos_working_dir="$1" #추후 수정

texecc()
{
        echo "[target]" $@;
        cd ${pos_working_dir}; sudo $@
}

result=`ps -ef | grep ../bin/poseidonos -c`

if [ ${result} -ne 0 ];
then
        texecc ./test/script/kill_poseidonos.sh
        echo "Could not exit properly"
        exit 1
fi
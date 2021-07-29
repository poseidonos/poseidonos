#!/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/../../

${ROOT_DIR}/bin/cli array unmount -a POSArray

${ROOT_DIR}/bin/cli system exit

timeelapsed=0
result=`ps -ef | grep bin/poseidonos | grep -v grep | wc -l`
while [ $result -gt 0 ];
do
    echo -en "Wait poseidonos exit " $timeelapsed "s\r"
    sleep 1
    timeelapsed=$((${timeelapsed} + 1))
    result=`ps -ef | grep bin/poseidonos | grep -v grep | wc -l`
done


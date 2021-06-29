#!/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/../../

${ROOT_DIR}/bin/cli array unmount -a POSArray

${ROOT_DIR}/bin/cli system exit

result=`ps -ef | grep bin/poseidonos | grep -v grep | wc -l`
while [ $result -gt 0 ];
do
    echo "Wait poseidonos exit"
    sleep 0.5
    result=`ps -ef | grep bin/poseidonos | grep -v grep | wc -l`
done


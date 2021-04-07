#!/bin/bash

IBOFOS_ROOT_DIR=$(readlink -f $(dirname $0))//../../../../..

if [ "$#" -lt 2 ] ; then
    echo "Usage: sh $0 [ ip addr ] [ git branch ]"
    exit
fi
TARGET_IP=$1
IBOFOS_GIT_BRANCH=$2

cd ${IBOFOS_ROOT_DIR}
git reset --hard
git checkout ${IBOFOS_GIT_BRANCH}

cd ${IBOFOS_ROOT_DIR}/lib/air/test/acceptance_test/overhead_test/
./overhead_test.sh ${TARGET_IP}

#!/bin/bash

IBOFOS_ROOT_DIR=$(readlink -f $(dirname $0))//../../../../../..

if [ "$#" -lt 1 ] ; then
    echo "Usage: sh $0 [ git branch ]"
    exit
fi
IBOFOS_GIT_BRANCH=$1

cd ${IBOFOS_ROOT_DIR}
git reset --hard
git checkout ${IBOFOS_GIT_BRANCH}

cd ${IBOFOS_ROOT_DIR}/lib/air/
make at_rsc_usage
cd test/acceptance_test/resource_usage/script/
taskset -c 9 ./measure_air_rsc.sh

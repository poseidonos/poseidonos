#!/bin/bash

TARGET_IP=10.100.11.6
PORT=1158

ROOT_DIR=$(readlink -f $(dirname $0)/../../../)
SPDK_DIR=${ROOT_DIR}/lib/spdk-19.10

RPC="sudo ${SPDK_DIR}/scripts/rpc.py"
CLI="sudo ${ROOT_DIR}/bin/cli"
DETACH="sudo ${ROOT_DIR}/test/script/detach_device.sh"
DATA_DEVICE_LIST="unvme-ns-0,unvme-ns-1,unvme-ns-2"
SPARE_DEVICE="unvme-ns-3"
DETACH_TARGET_DATA_1="unvme-ns-0"
DETACH_TARGET_DATA_2="unvme-ns-1"


ARRAY_NAME=POSArray

check_stopped()
{
    while [ `pgrep ibofos -c` -ne 0 ]
    do
        sleep 0.5
    done
}

start_ibofos()
{
    rm -rf /dev/shm/ibof_nvmf_trace.pid*
	echo 1 > /sys/bus/pci/rescan
    echo "iBoFOS starting..."
    ${ROOT_DIR}/test/regression/start_ibofos.sh

    result=`${CLI} request info --json | jq '.Response.info.state' 2>/dev/null`
    while [ -z ${result} ] || [ ${result} != '"OFFLINE"' ];
    do
        echo "Wait iBoFOS..."
        result=`${CLI} request info --json | jq '.Response.info.state' 2>/dev/null`
        sleep 0.5
    done
}

setup()
{
    echo "-------setup-------"
    ${RPC} nvmf_create_transport -t tcp -b 64 -n 4096
    ${RPC} bdev_malloc_create -b uram0 1024 512
    ${CLI} device scan
    ${RPC} nvmf_create_subsystem nqn.2019-04.ibof:subsystem1 -m 256 -a -s IBOF00000000000001 -d IBOF_VOLUME_EXTENTION
    ${RPC} nvmf_subsystem_add_listener nqn.2019-04.ibof:subsystem1 -t tcp -a ${TARGET_IP} -s ${PORT}
    ${CLI} array create -b uram0 -d ${DATA_DEVICE_LIST}
    ${CLI} system mount
    ${CLI} volume create --name vol1 --size 2147483648 --maxiops 0 --maxbw 0
    ${CLI} volume mount --name vol1
}

test()
{
    #IO
    ${DETACH} ${DETACH_TARGET_DATA_1} 1
    sleep 1
    ${CLI} device list
    ${CLI} array add -s ${SPARE_DEVICE}
    ${DETACH} ${DETACH_TARGET_DATA_2} 1
    ${CLI} system unmount
	${CLI} system info
	
	waiting_time=20
	for((i=${waiting_time};i>0;i--));
	do
		echo "Waiting to cancel rebuild. ${i} secondes left."
		sleep 1
	done

    ${CLI} system unmount
    ${CLI} system info
}

pkill -9 ibofos
check_stopped
start_ibofos
setup
test

echo "DONE"

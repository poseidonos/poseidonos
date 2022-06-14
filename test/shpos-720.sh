#!/bin/bash

ip="127.0.0.1"
logfile="../script/pos.log"

pause()
{
    echo "Press any key to continue.."
    read -rsn1
}

check_stopped()
{
	result=`texecc "pgrep poseidonos -c"`
    while [ `pgrep poseidonos -c` -ne 0 ]
    do
        echo "Waiting for POS stopped"
        sleep 0.5
    done
}

kill_pos()
{
    # kill pos if exists
    ./test/script/kill_poseidonos.sh 2>> ${logfile}
	check_stopped

    echo "PoseidonOS killed"
}

start_pos()
{
	rm -rf /dev/shm/ibof_nvmf_trace.pid*
	echo "PoseidonOS starting..."

    echo "Starting poseidonos..."
    ./regression/start_poseidonos.sh

    result=`../bin/../bin/poseidonos-cli system info --json-res | jq '.Response.info.version' 2>/dev/null`
	while [ -z ${result} ] || [ ${result} == '""' ];
	do
		echo "Wait PoseidonOS..."
		result=`../bin/../bin/poseidonos-cli system info --json-res | jq '.Response.info.version' 2>/dev/null`
		echo $result
		sleep 0.5
	done

    echo "Now poseidonos is running..."
}

print_title()
{
    local title=$1
    echo "===== $1 $(date +%H:%M:%S)"
}

print_title "start"
modprobe nvme-tcp
rm -rf /var/log/pos/*
start_pos;
# pause

../bin/poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem1 --serial-number IBOF00000000000001 --model-number IBOF_VOLUME_EXTENSION --max-namespaces 512 -o
../bin/poseidonos-cli subsystem create-transport --trtype tcp -c 64 --num-shared-buf 4096
../bin/poseidonos-cli subsystem add-listener --subnqn nqn.2019-04.ibof:subsystem1  --trtype tcp --traddr 127.0.0.1 --trsvcid 1158
../bin/poseidonos-cli subsystem list
../bin/poseidonos-cli device create --device-name uram0 --num-blocks 16777216  --block-size 512 --device-type "uram"
../bin/poseidonos-cli device scan
../bin/poseidonos-cli devel resetmbr
../bin/poseidonos-cli logger set-level --level debug

../bin/poseidonos-cli array autocreate -a ARRAY1 -b uram0 -r RAID0 -d 2
../bin/poseidonos-cli array mount -a ARRAY1
pause
../bin/poseidonos-cli volume create -a ARRAY1 -v vol1 --size 20GB
../bin/poseidonos-cli volume mount -a ARRAY1 -v vol1
../bin/poseidonos-cli volume unmount -a ARRAY1 -v vol1 --force
../bin/poseidonos-cli array unmount -a ARRAY1 --force

../bin/poseidonos-cli array mount -a ARRAY1 -w
pause
../bin/poseidonos-cli volume mount -a ARRAY1 -v vol1
../bin/poseidonos-cli volume unmount -a ARRAY1 -v vol1 --force
../bin/poseidonos-cli array unmount -a ARRAY1 --force

../bin/poseidonos-cli array mount -a ARRAY1
../bin/poseidonos-cli volume mount -a ARRAY1 -v vol1
../bin/poseidonos-cli volume unmount -a ARRAY1 -v vol1 --force
../bin/poseidonos-cli array unmount -a ARRAY1 --force

../bin/poseidonos-cli array mount -a ARRAY1 -w
../bin/poseidonos-cli volume mount -a ARRAY1 -v vol1
../bin/poseidonos-cli volume unmount -a ARRAY1 -v vol1 --force
../bin/poseidonos-cli array unmount -a ARRAY1 --force

nvme disconnect -n nqn.2019-04.ibof:subsystem1
../bin/poseidonos-cli system stop --force

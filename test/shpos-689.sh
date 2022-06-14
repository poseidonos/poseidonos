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

    result=`../bin/poseidonos-cli system info --json-res | jq '.Response.info.version' 2>/dev/null`
	while [ -z ${result} ] || [ ${result} == '""' ];
	do
		echo "Wait PoseidonOS..."
		result=`../bin/poseidonos-cli system info --json-res | jq '.Response.info.version' 2>/dev/null`
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
start_pos;
# pause

print_title "1"
../bin/poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem1 --serial-number IBOF00000000000001 --model-number IBOF_VOLUME_EXTENSION --max-namespaces 512 -o

print_title "2"
../bin/poseidonos-cli device create --device-name uram0 --num-blocks 16777216  --block-size 512 --device-type "uram"
../bin/poseidonos-cli device scan
../bin/poseidonos-cli devel resetmbr
pause

print_title "3"
../bin/poseidonos-cli array create --array-name POSARRAY1 --buffer uram0 --data-devs unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3,unvme-ns-4 --raid RAID0
# pause

../bin/poseidonos-cli array mount --array-name POSARRAY1
# pause

print_title "4"
../bin/poseidonos-cli volume create -v ibof_vol_1 --size 1gb -a POSARRAY1
# pause

print_title "5"
../bin/poseidonos-cli volume delete -a POSARRAY1 -v ibof_vol_1 --force

print_title "6"
../bin/poseidonos-cli array unmount --array-name POSARRAY1 --force

print_title "done"
../bin/poseidonos-cli system stop --force

#!/bin/bash

ip="127.0.0.1"
port=1158
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
    echo "===== $1 $(date +%H:%M:%S)"
}

print_title "start"
modprobe nvme-tcp
rm -rf /var/log/pos/*
start_pos;
# pause

loop_count=500
array_name="POSArray1"
volume_count=12
volume_size=8

../bin/poseidonos-cli subsystem list
../bin/poseidonos-cli device create --device-name uram0 --num-blocks 16777216  --block-size 512 --device-type "uram"
../bin/poseidonos-cli device scan
../bin/poseidonos-cli devel resetmbr
../bin/poseidonos-cli logger set-level --level debug
../bin/poseidonos-cli array autocreate -a $array_name -b uram0 -r RAID0 -d 12

create=0
nvme_connect=0

for i in $( seq 1 $volume_count )
do
    nvme disconnect -n nqn.2019-04.ibof:subsystem$i
done

for loop in $( seq 1 $loop_count )
do
    print_title "loop: $loop"
    echo "mount array"
    ../bin/poseidonos-cli array mount -a $array_name

    if [ $create == 0 ]; then
        echo "create volumes"
        for i in $( seq 1 $volume_count )
        do
            ../bin/poseidonos-cli volume create -a $array_name -v vol$i --size ${volume_size}GB
            ../bin/poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem$i --serial-number IBOF0000000000000$i --model-number IBOF_VOLUME_EXTENSION --max-namespaces 512 -o
        done
        ../bin/poseidonos-cli subsystem create-transport --trtype tcp -c 64 --num-shared-buf 4096
        create=1
    fi

    echo "mount volumes"
    for i in $( seq 1 $volume_count )
    do
        ../bin/poseidonos-cli volume mount -a $array_name -v vol$i --subnqn nqn.2019-04.ibof:subsystem$i --force
    done

    if [ $nvme_connect == 0 ]; then
        for i in $( seq 1 $volume_count )
        do
            ../bin/poseidonos-cli subsystem add-listener --subnqn nqn.2019-04.ibof:subsystem$i --trtype tcp --traddr $ip --trsvcid $port
            nvme connect -t tcp -s $port -a $ip -n nqn.2019-04.ibof:subsystem$i
            nvme_connect=1
        done
    fi

    fio_string=""

    for (( i = 0; i < $volume_count; i++))
    do
        fio_string="${fio_string} --name=test_$i --verify=0 --ioengine=aio --iodepth=128 --io_size=${volume_size}g --rw=write --bs=128k --filename=/dev/nvme${i}n1"
    done
    # echo $fio_string
    # pause
    fio $fio_string

    echo "unmount volumes"
    for i in $( seq 1 $volume_count )
    do
        ../bin/poseidonos-cli volume unmount -a $array_name -v vol$i --force
    done

    echo "unmount array"
    ../bin/poseidonos-cli array unmount -a $array_name --force
done

for i in $( seq 1 $volume_count )
do
    nvme disconnect -n nqn.2019-04.ibof:subsystem$i
done

../bin/poseidonos-cli system stop --force

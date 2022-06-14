#!/bin/bash

ip="10.1.3.30"
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

pkill -9 poseidonos
sleep 3

print_title "start"
nvme disconnect -n nqn.2019-04.ibof:subsystem1
nvme disconnect -n nqn.2019-04.ibof:subsystem2
rm -rf /var/log/pos/*
modprobe nvme-tcp
start_pos;

print_title "create array1"
../bin/poseidonos-cli subsystem create-transport --trtype tcp -c 64 --num-shared-buf 4096
../bin/poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem1 --serial-number IBOF00000000000001 --model-number IBOF_VOLUME_EXTENSION --max-namespaces 512 -o
../bin/poseidonos-cli subsystem list
../bin/poseidonos-cli device create --device-name uram0 --num-blocks 8388608  --block-size 512 --device-type "uram"
../bin/poseidonos-cli device scan
../bin/poseidonos-cli devel resetmbr
../bin/poseidonos-cli array create --array-name POSARRAY1 --buffer uram0 --data-devs unvme-ns-0,unvme-ns-1,unvme-ns-2 --spare unvme-ns-3 --raid RAID5
../bin/poseidonos-cli array mount --array-name POSARRAY1 -w

print_title "create a volume on array1"
../bin/poseidonos-cli volume create -v ibof_vol_1 --size 1tb -a POSARRAY1
../bin/poseidonos-cli volume mount -v ibof_vol_1 -a POSARRAY1 --subnqn nqn.2019-04.ibof:subsystem1 --force
../bin/poseidonos-cli subsystem add-listener --subnqn nqn.2019-04.ibof:subsystem1 --trtype tcp --traddr 127.0.0.1 --trsvcid 1158
nvme connect -t tcp -s 1158 -a 127.0.0.1 -n nqn.2019-04.ibof:subsystem1

print_title "create array2"
../bin/poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem2 --serial-number IBOF00000000000002 --model-number IBOF_VOLUME_EXTENSION --max-namespaces 512 -o
../bin/poseidonos-cli subsystem list
../bin/poseidonos-cli device create --device-name uram1 --num-blocks 8388608  --block-size 512 --device-type "uram"
../bin/poseidonos-cli device scan
../bin/poseidonos-cli array create --array-name POSARRAY2 --buffer uram1 --data-devs unvme-ns-4,unvme-ns-5,unvme-ns-6 --spare unvme-ns-7 --raid RAID5
../bin/poseidonos-cli array mount --array-name POSARRAY2

print_title "create a volume on array2"
../bin/poseidonos-cli volume create -v ibof_vol_1 --size 1tb -a POSARRAY2
../bin/poseidonos-cli volume mount -v ibof_vol_1 -a POSARRAY2 --subnqn nqn.2019-04.ibof:subsystem2 --force
../bin/poseidonos-cli subsystem add-listener --subnqn nqn.2019-04.ibof:subsystem2 --trtype tcp --traddr 127.0.0.1 --trsvcid 1158
nvme connect -t tcp -s 1158 -a 127.0.0.1 -n nqn.2019-04.ibof:subsystem2

print_title "io"
# sleep 1
# ls -al /dev/nvme*
# fio --name=test_0 --verify=0 --ioengine=aio --iodepth=128 --io_size=1g --rw=write --bs=128k --filename=/dev/nvme0n1 \
#     --name=test_1 --verify=0 --ioengine=aio --iodepth=128 --io_size=1g --rw=write --bs=128k --filename=/dev/nvme1n1

print_title "spo"
pkill -9 poseidonos
sleep 3
../script/backup_latest_hugepages_for_uram.sh

print_title "bring up"
start_pos;
../bin/poseidonos-cli subsystem list
../bin/poseidonos-cli device create --device-name uram0 --num-blocks 8388608  --block-size 512 --device-type "uram"
../bin/poseidonos-cli device create --device-name uram1 --num-blocks 8388608  --block-size 512 --device-type "uram"
../bin/poseidonos-cli device scan

print_title "mount array1"
../bin/poseidonos-cli array mount --array-name POSARRAY1 -w

print_title "mount array2"
../bin/poseidonos-cli array mount --array-name POSARRAY2

print_title "shutdown"
nvme disconnect -n nqn.2019-04.ibof:subsystem1
nvme disconnect -n nqn.2019-04.ibof:subsystem2
../bin/poseidonos-cli array unmount --array-name POSARRAY1
../bin/poseidonos-cli array unmount --array-name POSARRAY2
../bin/poseidonos-cli system stop --force

print_title "done"

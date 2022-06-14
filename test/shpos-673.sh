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

# print_title "1"
# ../bin/poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem1 --serial-number IBOF00000000000001 --model-number IBOF_VOLUME_EXTENSION --max-namespaces 512 -o
# ../bin/poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem2 --serial-number IBOF00000000000002 --model-number IBOF_VOLUME_EXTENSION --max-namespaces 512 -o
# ../bin/poseidonos-cli subsystem list

# print_title "2"
# ../bin/poseidonos-cli device create --device-name uram0 --num-blocks 16777216  --block-size 512 --device-type "uram"
# ../bin/poseidonos-cli device create --device-name uram1 --num-blocks 16777216  --block-size 512 --device-type "uram"
# ../bin/poseidonos-cli device scan
# ../bin/poseidonos-cli device list
# ../bin/poseidonos-cli devel resetmbr

# print_title "3"
# ../bin/poseidonos-cli array create --array-name POSARRAY1 --buffer uram0 --data-devs unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3 --spare unvme-ns-4 --raid RAID5
# ../bin/poseidonos-cli array mount --array-name POSARRAY1

# print_title "4"
# ../bin/poseidonos-cli array create --array-name POSARRAY2 --buffer uram1 --data-devs unvme-ns-5,unvme-ns-6,unvme-ns-7,unvme-ns-8 --spare unvme-ns-9 --raid RAID5
# ../bin/poseidonos-cli array mount --array-name POSARRAY2
# ../bin/poseidonos-cli array list
# pause

# print_title "5"
# for i in {1..2}
# do
#     sudo ../bin/poseidonos-cli volume create -v ibof_vol_$i --size 1tb -a POSARRAY1
#     pause
#     sudo ../bin/poseidonos-cli volume mount -v ibof_vol_$i -a POSARRAY1 --subnqn nqn.2019-04.ibof:subsystem1 --force
# done

# print_title "6"
# for i in {1..2}
# do
#     sudo ../bin/poseidonos-cli volume create -v ibof_vol_$i --size 1tb -a POSARRAY2
#     pause
#     sudo ../bin/poseidonos-cli volume mount -v ibof_vol_$i -a POSARRAY2 --subnqn nqn.2019-04.ibof:subsystem2 --force
# done

# print_title "7"
# ../bin/poseidonos-cli subsystem create-transport --trtype tcp -c 64 --num-shared-buf 4096
# ../bin/poseidonos-cli subsystem add-listener --subnqn nqn.2019-04.ibof:subsystem1  --trtype tcp --traddr 127.0.0.1 --trsvcid 1158
# ../bin/poseidonos-cli subsystem add-listener --subnqn nqn.2019-04.ibof:subsystem2  --trtype tcp --traddr 127.0.0.1 --trsvcid 1158
# nvme connect -t tcp -s 1158 -a ${ip} -n nqn.2019-04.ibof:subsystem1
# nvme connect -t tcp -s 1158 -a ${ip} -n nqn.2019-04.ibof:subsystem2
# # pause

# print_title "8"
# # fio --name=test_1 --ioengine=aio --iodepth=32 --rw=rw --size=1gb --bs=4kb --numjobs=4  --fdatasync=32 --filename='trtype=tcp adrfam=IPv4 traddr=127.0.0.1 trsvcid=1158 subnqn=nqn.2019-04.ibof\:subsystem1 ns=1'
# # fio --name=test_2 --ioengine=aio --iodepth=32 --rw=rw --size=1gb --bs=4kb --numjobs=4  --fdatasync=32 --filename='trtype=tcp adrfam=IPv4 traddr=127.0.0.1 trsvcid=1158 subnqn=nqn.2019-04.ibof\:subsystem1 ns=2'
# # fio --name=test_3 --ioengine=aio --iodepth=32 --rw=rw --size=1gb --bs=4kb --numjobs=4  --fdatasync=32 --filename='trtype=tcp adrfam=IPv4 traddr=127.0.0.1 trsvcid=1158 subnqn=nqn.2019-04.ibof\:subsystem2 ns=1'
# # fio --name=test_4 --ioengine=aio --iodepth=32 --rw=rw --size=1gb --bs=4kb --numjobs=4  --fdatasync=32 --filename='trtype=tcp adrfam=IPv4 traddr=127.0.0.1 trsvcid=1158 subnqn=nqn.2019-04.ibof\:subsystem2 ns=2'
# fio --name=test_1 --verify=0 --ioengine=aio --runtime=600 --iodepth=32 --io_size=1t --rw=write --bs=128k --filename=/dev/nvme0n1 \
#  --name=test_2 --verify=0 --ioengine=aio --runtime=600 --iodepth=32 --io_size=1t --rw=write -bs=128k --filename=/dev/nvme0n2 \
#  --name=test_3 --verify=0 --ioengine=aio --runtime=600 --iodepth=32 --io_size=1t --rw=write --bs=128k --filename=/dev/nvme1n1 \
#  --name=test_4 --verify=0 --ioengine=aio --runtime=600 --iodepth=32 --io_size=1t --rw=write --bs=128k --filename=/dev/nvme1n2

# print_title "9. kill"
# # for i in {1..2}
# # do
# #     sudo ../bin/poseidonos-cli volume unmount -v ibof_vol_$i -a POSARRAY2 --force
# # done
# # ../bin/poseidonos-cli array unmount --array-name POSARRAY1 --force
# # ../bin/poseidonos-cli array unmount --array-name POSARRAY2 --force
# # ../bin/poseidonos-cli system stop --force
# # nvme disconnect -n nqn.2019-04.ibof:subsystem1
# # nvme disconnect -n nqn.2019-04.ibof:subsystem2
# # sleep 120
# pkill -9 poseidonos
# sleep 3
# ../script/backup_latest_hugepages_for_uram.sh

print_title "10. restart"
start_pos;

print_title "11"
../bin/poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem1 --serial-number IBOF00000000000001 --model-number IBOF_VOLUME_EXTENSION --max-namespaces 512 -o
../bin/poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem2 --serial-number IBOF00000000000002 --model-number IBOF_VOLUME_EXTENSION --max-namespaces 512 -o
../bin/poseidonos-cli subsystem list

print_title "12"
../bin/poseidonos-cli device create --device-name uram0 --num-blocks 16777216  --block-size 512 --device-type "uram"
../bin/poseidonos-cli device create --device-name uram1 --num-blocks 16777216  --block-size 512 --device-type "uram"
../bin/poseidonos-cli device scan
../bin/poseidonos-cli device list
# ../bin/poseidonos-cli devel resetmbr

print_title "13"
../bin/poseidonos-cli array mount --array-name POSARRAY1

print_title "14"
# ../bin/poseidonos-cli array mount --array-name POSARRAY2
# ../bin/poseidonos-cli array list

# print_title "15"
# for i in {1..2}
# do
#     sudo ../bin/poseidonos-cli volume mount -v ibof_vol_$i -a POSARRAY1 --subnqn nqn.2019-04.ibof:subsystem1 --force
#     sudo ../bin/poseidonos-cli volume mount -v ibof_vol_$i -a POSARRAY2 --subnqn nqn.2019-04.ibof:subsystem2 --force
# done

# print_title "16"
# ../bin/poseidonos-cli array unmount --array-name POSARRAY1 --force
# ../bin/poseidonos-cli array unmount --array-name POSARRAY2 --force

# print_title "done"
# ../bin/poseidonos-cli system stop --force
# nvme disconnect -n nqn.2019-04.ibof:subsystem1
# nvme disconnect -n nqn.2019-04.ibof:subsystem2

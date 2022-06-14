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
start_pos;
# pause

print_title "1. Create subsystem and uram:"
../bin/poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem1 --serial-number IBOF00000000000001 --model-number IBOF_VOLUME_EXTENSION --max-namespaces 512 -o
# ../bin/poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem2 --serial-number IBOF00000000000002 --model-number IBOF_VOLUME_EXTENSION --max-namespaces 512 -o
../bin/poseidonos-cli subsystem create-transport --trtype tcp -c 64 --num-shared-buf 4096
../bin/poseidonos-cli subsystem add-listener --subnqn nqn.2019-04.ibof:subsystem1  --trtype tcp --traddr 127.0.0.1 --trsvcid 1158
# ../bin/poseidonos-cli subsystem add-listener --subnqn nqn.2019-04.ibof:subsystem2  --trtype tcp --traddr 127.0.0.1 --trsvcid 1158
../bin/poseidonos-cli subsystem list
../bin/poseidonos-cli device create --device-name uram0 --num-blocks 16777216  --block-size 512 --device-type "uram"
# ../bin/poseidonos-cli device create --device-name uram1 --num-blocks 16777216  --block-size 512 --device-type "uram"
../bin/poseidonos-cli device scan
../bin/poseidonos-cli devel resetmbr
# pause

print_title "2. Create Array and volume:"
# ../bin/poseidonos-cli array autocreate -a ARRAY2 -b uram0 --no-raid -d 1
# ../bin/poseidonos-cli array autocreate -a ARRAY2 -b uram0 -r RAID0 -d 2
# ../bin/poseidonos-cli array autocreate -a ARRAY2 -b uram1 -r RAID0 -d 2
# ../bin/poseidonos-cli array mount -a ARRAY2
# ../bin/poseidonos-cli array autocreate -a ARRAY1 -b uram1 -r RAID0 -d 2
../bin/poseidonos-cli array autocreate -a ARRAY1 -b uram0 -r RAID0 -d 2
../bin/poseidonos-cli array mount -a ARRAY1
../bin/poseidonos-cli volume create -a ARRAY1 -v vol1 --size 20GB
../bin/poseidonos-cli volume create -a ARRAY1 -v vol2 --size 20GB
../bin/poseidonos-cli volume mount -a ARRAY1 -v vol1
../bin/poseidonos-cli volume mount -a ARRAY1 -v vol2
# pause

print_title "3. From Initiator side:"
nvme connect -t tcp -s 1158 -a 127.0.0.1 -n nqn.2019-04.ibof:subsystem1
# nvme connect -t tcp -s 1158 -a 127.0.0.1 -n nqn.2019-04.ibof:subsystem2
sleep 1
pause

print_title "4. Create FS, mount FS and create file"
mkfs.xfs /dev/nvme0n1
mkdir /mnt/sat
mount /dev/nvme0n1 /mnt/sat
df -h
pause

print_title "5. Create files using dd command and fill the capacity"
# dd if=/dev/zero of=file1 bs=1024 count=100000
# pause

print_title "6. Check md5sum of file"
# ls
# md5sum file1
# pause

print_title "7. unmount mountpoint"
umount /mnt/sat
# pause

print_title "8. Trigger SPOR sequence:"
pkill -9 poseidonos
sleep 3
../script/backup_latest_hugepages_for_uram.sh
nvme disconnect -n nqn.2019-04.ibof:subsystem1
# nvme disconnect -n nqn.2019-04.ibof:subsystem2
# ../bin/poseidonos-cli volume unmount -a ARRAY1 -v vol1 --force
# ../bin/poseidonos-cli volume unmount -a ARRAY1 -v vol2 --force
# ../bin/poseidonos-cli array unmount -a ARRAY1 --force
# ../bin/poseidonos-cli array unmount -a ARRAY2 --force
# ../bin/poseidonos-cli system stop --force
# sleep 90

print_title "9. Start POS"
start_pos;
# pause

print_title "11. Create subsystem and uram:"
../bin/poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem1 --serial-number IBOF00000000000001 --model-number IBOF_VOLUME_EXTENSION --max-namespaces 512 -o
# ../bin/poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem2 --serial-number IBOF00000000000002 --model-number IBOF_VOLUME_EXTENSION --max-namespaces 512 -o
../bin/poseidonos-cli subsystem list
../bin/poseidonos-cli device create --device-name uram0 --num-blocks 16777216  --block-size 512 --device-type "uram"
# ../bin/poseidonos-cli device create --device-name uram1 --num-blocks 16777216  --block-size 512 --device-type "uram"
../bin/poseidonos-cli device scan
../bin/poseidonos-cli device list
../bin/poseidonos-cli array list
../bin/poseidonos-cli array mount -a ARRAY1
# ../bin/poseidonos-cli array mount -a ARRAY2
../bin/poseidonos-cli subsystem create-transport --trtype tcp -c 64 --num-shared-buf 4096
../bin/poseidonos-cli subsystem add-listener --subnqn nqn.2019-04.ibof:subsystem1  --trtype tcp --traddr 127.0.0.1 --trsvcid 1158
# ../bin/poseidonos-cli subsystem add-listener --subnqn nqn.2019-04.ibof:subsystem2  --trtype tcp --traddr 127.0.0.1 --trsvcid 1158
../bin/poseidonos-cli volume list -a ARRAY1
pause

print_title "12. Mount volume into Array"
../bin/poseidonos-cli volume mount -a ARRAY1 -v vol1
../bin/poseidonos-cli volume mount -a ARRAY1 -v vol2
../bin/poseidonos-cli volume list -a ARRAY1
pause

print_title "13. Initiator side: Do nvme connect"
nvme connect -t tcp -s 1158 -a 127.0.0.1 -n nqn.2019-04.ibof:subsystem1
# nvme connect -t tcp -s 1158 -a 127.0.0.1 -n nqn.2019-04.ibof:subsystem2
sleep 1
# pause

print_title "14. Mount device"
mount /dev/nvme0n1 /mnt/sat
pause

print_title "done"
umount /mnt/sat
../bin/poseidonos-cli volume unmount -a ARRAY1 -v vol1 --force
# ../bin/poseidonos-cli volume unmount -a ARRAY1 -v vol2 --force
../bin/poseidonos-cli array unmount -a ARRAY1 --force
# ../bin/poseidonos-cli array unmount -a ARRAY2 --force
nvme disconnect -n nqn.2019-04.ibof:subsystem1
nvme disconnect -n nqn.2019-04.ibof:subsystem2
../bin/poseidonos-cli system stop --force

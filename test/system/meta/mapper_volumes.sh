#!/bin/bash
## This script is for VM

## Variables
rootdir=$(readlink -f $(dirname $0))/../../..
spdkdir=$rootdir/lib/spdk
# fiodir=$rootdir/test/system/nvmf/initiator
fiodir=$rootdir/test/system/io_path

fablic_ip=10.100.11.26
port=1158

numVolumes=4
perVolSizeinGB=25
MBtoB=$((1024*1024))
GBtoB=$((1024*${MBtoB}))
perVolSize=$(expr $perVolSizeinGB \* $GBtoB)


## functions
# Common until array handling

ibofos_forced_kill() {
    sudo $rootdir/test/script/kill_poseidonos.sh
}

ibofos_bringup() {
    sudo $rootdir/script/start_poseidonos.sh
    sleep 10
}

common_until_array() {
    sudo $spdkdir/scripts/rpc.py nvmf_create_transport -t TCP -b 64 -n 4096
    sudo $spdkdir/scripts/rpc.py bdev_malloc_create -b uram0 1024 512
    sudo $rootdir/bin/cli request scan_dev

    # This test uses 4 volumes
    sudo $spdkdir/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem1 -a -s POS00000000000001 -d POS_VOLUME_EXTENTION
    sudo $spdkdir/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem2 -a -s POS00000000000002 -d POS_VOLUME_EXTENTION
    sudo $spdkdir/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem3 -a -s POS00000000000003 -d POS_VOLUME_EXTENTION
    sudo $spdkdir/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem4 -a -s POS00000000000004 -d POS_VOLUME_EXTENTION

    sudo $spdkdir/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem1 -t tcp -a $fablic_ip -s $port
    sudo $spdkdir/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem2 -t tcp -a $fablic_ip -s $port
    sudo $spdkdir/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem3 -t tcp -a $fablic_ip -s $port
    sudo $spdkdir/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem4 -t tcp -a $fablic_ip -s $port

    sudo $spdkdir/scripts/rpc.py nvmf_get_subsystems

    sudo $rootdir/bin/cli request set_log_level --level info
}

array_create() {
    # WriteBuffer: uram0 
    # DataStorage: unvme-ns-0 ... 2
    # Spare: unvme-ns-3
    sudo $rootdir/bin/cli devel resetmbr
    sudo $rootdir/bin/cli request create_array -b uram0 -d unvme-ns-0,unvme-ns-1,unvme-ns-2 -s unvme-ns-3
}

ibofos_mount() {
    sudo $rootdir/bin/cli request mount_ibofos
}

volume_create() {
    sudo $rootdir/bin/cli request create_vol --name vol1 --size $perVolSize --maxbw 0
    sudo $rootdir/bin/cli request create_vol --name vol2 --size $perVolSize --maxbw 0
    sudo $rootdir/bin/cli request create_vol --name vol3 --size $perVolSize --maxbw 0
    sudo $rootdir/bin/cli request create_vol --name vol4 --size $perVolSize --maxbw 0
}

volume_all_mount() {
    sudo $rootdir/bin/cli request mount_vol --name vol1 --subnqn nqn.2019-04.pos:subsystem1
    sudo $rootdir/bin/cli request mount_vol --name vol2 --subnqn nqn.2019-04.pos:subsystem2 
    sudo $rootdir/bin/cli request mount_vol --name vol3 --subnqn nqn.2019-04.pos:subsystem3 
    sudo $rootdir/bin/cli request mount_vol --name vol4 --subnqn nqn.2019-04.pos:subsystem4 
}

a_volume_mount() {
    sleep $2
    sudo $rootdir/bin/cli request mount_vol --name vol$1 --subnqn nqn.2019-04.pos:subsystem$1
}

seqwrite_all_volume()
{
    # $fiodir/fio_full_bench.py --traddr=$fablic_ip --trtype=tcp --readwrite=write --io_size=${perVolSizeinGB}G --verify=false --bs=128k --time_based=0 --run_time=0 --iodepth=32 --file_num=4
    $fiodir/fio_bench.py --traddr=$fablic_ip --trtype=tcp --readwrite=write --io_size=${perVolSizeinGB}G --verify=false --bs=128k --time_based=0 --run_time=0 --iodepth=32 --file_num=4
}

# 5GB Write to volume 1 and 2: seqwrite_volume1_2 5
seqwrite_volume1_2()
{
    # $fiodir/fio_full_bench.py --traddr=$fablic_ip --trtype=tcp --readwrite=write --io_size=$1G --verify=false --bs=128k --time_based=0 --run_time=0 --iodepth=32 --file_num=2
    $fiodir/fio_bench.py --traddr=$fablic_ip --trtype=tcp --readwrite=write --io_size=$1G --verify=false --bs=128k --time_based=0 --run_time=0 --iodepth=32 --file_num=2
}

# 5GB write to Volume 1: seqwrite_volume_n 5 1
seqwrite_volume_n() 
{
    $fiodir/fio_bench.py --traddr=$fablic_ip --trtype=tcp --readwrite=write --io_size=$1G --verify=false --bs=128k --time_based=0 --run_time=0 --iodepth=32 --target_volume=$2
}

shutdown_ibofos()
{
    sudo $rootdir/bin/cli request unmount_ibofos
    sudo $rootdir/bin/cli request exit_ibofos
    sleep 10
    sudo $rootdir/bin/cli request exit_ibofos
}


## Test Sequence start
ibofos_forced_kill

# create 4 volumes > mount them > shutdown
ibofos_bringup
common_until_array
array_create
ibofos_mount
volume_create
volume_all_mount
shutdown_ibofos

# mount volume 1, 2 > Write I/O volume 1, 2 > shutdown
#                        mount volume 3
ibofos_bringup
common_until_array
ibofos_mount
a_volume_mount 1 0
a_volume_mount 2 0
a_volume_mount 3 7 &    # 7sec later, mount vol3
seqwrite_volume1_2 3    # 3GB Write to volume 1 and 2
shutdown_ibofos

# mount volume 3, 4 > Write I/O volume 3 > shutdown
#                       mount volume 1
ibofos_bringup
common_until_array
ibofos_mount
a_volume_mount 3 0
a_volume_mount 4 0
a_volume_mount 1 7 &    # 7sec later, mount vol1
seqwrite_volume_n 5 3   # 5GB Write to volume 3
shutdown_ibofos

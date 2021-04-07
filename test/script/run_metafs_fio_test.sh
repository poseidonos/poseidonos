#!/bin/bash

# Prerequisite:
#  - build ibofos with WBT option (./configure --with-wbt && make -j32)
#  
# How to use:
#  1) run ibofos (./bin/ibofos)
#  2) configure fabric network on target side and change this script accordingly
#  3) run ./run_metafs_fio_test.sh

trtype=tcp
volume_size=21474836480
metafs_testfile_size=$((384*1024*1024)) # $((${volume_size}/16))
target_fabric_ip=172.16.1.1
port=1158
volname="bdev"
NR_VOLUME=1
SUBSYSTEM_NUM=1
PORT_NUM=1

./start_ibofos.sh
sleep 5

sudo ../lib/spdk-19.10/scripts/rpc.py nvmf_create_transport -t ${trtype} -b 64 -n 4096
sudo ../lib/spdk-19.10/scripts/rpc.py bdev_malloc_create -b uram0 4096 512

sudo ../bin/cli request scan_dev
sudo ../bin/cli request create_array -b uram0 -d unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3,unvme-ns-4,unvme-ns-5,unvme-ns-6,unvme-ns-7
sudo ../bin/cli request mount_ibofos

for i in `seq 1 $SUBSYSTEM_NUM`
do
    sudo ../lib/spdk-19.10/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.ibof:subsystem$i -m 256 -a -s IBOF0000000000000$i -d IBOF_VOL_$i
    sudo ../lib/spdk-19.10/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.ibof:subsystem$i -t ${trtype} -a ${target_fabric_ip} -s ${port}
done

for i in `seq 1 $NR_VOLUME`
do
    volIndex=`expr $i - 1`
    sudo ../bin/cli request create_vol --name ${volname}$volIndex --size ${volume_size} --maxiops 0 --maxbw 0
    sudo ../bin/cli request mount_vol --name ${volname}$volIndex

    sudo nvme connect -t tcp -n nqn.2019-04.ibof:subsystem$i -a ${target_fabric_ip} -s 1158
done

sudo ../lib/spdk-19.10/scripts/rpc.py nvmf_get_subsystems

sudo ../bin/cli wbt --json mfs_dump_files_list -o file_list.txt
fdList=`sudo cat ./file_list.txt | jq '.filesInfoList[] | select(.fileName | test("VSAMap")?) | .fd'`
sudo rm -rf ./file_list.txt

for i in $fdList; do
    sudo ../bin/cli wbt mfs_setup_meta_fio_test --name ${volname} --size $i
done

sleep 2s;

sudo ./fio_meta_bench.py -t ${trtype} -i ${target_fabric_ip} -p ${port} -n 1

sudo ./kill_ibofos.sh

for i in `seq 1 $NR_VOLUME`
do
    sudo nvme disconnect -n nqn.2019-04.ibof:subsystem$i
done

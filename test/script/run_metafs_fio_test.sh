#!/bin/bash

# Prerequisite:
#  - build poseidonos with WBT option (./configure --with-wbt && make -j32)
#
# How to use:
#  1) run ./run_metafs_fio_test.sh param
#
# param: on virtual machine or not => ./run_metafs_fio_test.sh vm

pause()
{
    echo "Press any key to continue.."
    read -rsn1
}

vm=$1
trtype=tcp
volume_size=21474836480
metafs_testfile_size=$((${volume_size}/512))
port=1158
volname="bdev"
NR_VOLUME=2 # right now, max=4
SUBSYSTEM_NUM=$NR_VOLUME

../../script/start_poseidonos.sh
sleep 5

if [ "$vm" == "vm" ]; then
    echo "################################"
    echo "# running on a virtual machine #"
    echo "################################"
    sleep 1

    target_fabric_ip=10.100.11.10

    sudo ../../lib/spdk/scripts/rpc.py nvmf_create_transport -t ${trtype} -b 64 -n 2048
    sudo ../../lib/spdk/scripts/rpc.py bdev_malloc_create -b uram0 2048 512

    sudo ../../bin/cli device scan
    sudo ../../bin/cli array create -b uram0 -d unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3 --name POSArray
else
    target_fabric_ip=172.16.1.1

    sudo ../../lib/spdk/scripts/rpc.py nvmf_create_transport -t ${trtype} -b 64 -n 2048
    sudo ../../lib/spdk/scripts/rpc.py bdev_malloc_create -b uram0 4096 512

    sudo ../../bin/cli device scan
    sudo ../../bin/cli array create -b uram0 -d unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3,unvme-ns-4,unvme-ns-5,unvme-ns-6,unvme-ns-7 --name POSArray
fi

sudo ../../bin/cli system mount

for i in `seq 1 $SUBSYSTEM_NUM`
do
    sudo ../../lib/spdk/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem$i -m 256 -a -s POS0000000000000$i -d POS_VOL_$i
    sudo ../../lib/spdk/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem$i -t ${trtype} -a ${target_fabric_ip} -s ${port}
done

for i in `seq 1 $NR_VOLUME`
do
    volIndex=`expr $i - 1`
    sudo ../../bin/cli volume create --name ${volname}$volIndex --size ${volume_size} --maxiops 0 --maxbw 0 --array POSArray
done

sudo ../../lib/spdk/scripts/rpc.py nvmf_get_subsystems

sudo ../../bin/cli wbt mfs_dump_files_list -o file_list.txt
fdList=`sudo cat ./file_list.txt | jq '.filesInfoList[] | select(.fileName | test("VSAMap")?) | .fd'`
sudo rm -rf ./file_list.txt

for i in $fdList;
do
    sudo ../../bin/cli wbt mfs_setup_meta_fio_test --name ${volname} --size $i
done

for i in `seq 1 $NR_VOLUME`
do
    volIndex=`expr $i - 1`
    sudo ../../bin/cli volume mount --name ${volname}$volIndex --array POSArray

    sudo nvme connect -t tcp -n nqn.2019-04.pos:subsystem$i -a ${target_fabric_ip} -s 1158
done

sleep 2s;

sudo ./fio_meta_bench.py -t ${trtype} -i ${target_fabric_ip} -p ${port} -n ${NR_VOLUME}

for i in `seq 1 $NR_VOLUME`
do
    sudo nvme disconnect -n nqn.2019-04.pos:subsystem$i
done

sudo ./kill_ibofos.sh

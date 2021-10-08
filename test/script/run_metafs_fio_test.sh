#!/bin/bash

# Prerequisite:
#  - build poseidonos with WBT option (./configure --with-wbt && make -j32)
#
# How to use:
#  1) run ./run_metafs_fio_test.sh param

ROOT_DIR=$(readlink -f $(dirname $0))/../..

pause()
{
    echo "Press any key to continue.."
    read -rsn1
}

while getopts "f:w:" opt
do
    case "$opt" in
        f) ip="$OPTARG"
            ;;
        w) wbt="$OPTARG"
    esac
done

vm=$1
volume_size=2147483648
metafs_testfile_size=$((${volume_size}/512))
port=1158
volname="bdev"
NR_VOLUME=4 # right now, max=4
SUBSYSTEM_NUM=$NR_VOLUME

sudo ${ROOT_DIR}/test/script/kill_poseidonos.sh
sleep 3
sudo ${ROOT_DIR}/test/regression/start_poseidonos.sh
sleep 5

sudo ${ROOT_DIR}/lib/spdk/scripts/rpc.py nvmf_create_transport -t TCP -b 64 -n 4096
sudo ${ROOT_DIR}/lib/spdk/scripts/rpc.py bdev_malloc_create -b uram0 1024 512

sudo ${ROOT_DIR}/bin/poseidonos-cli device scan
sudo ${ROOT_DIR}/bin/poseidonos-cli devel resetmbr

sudo ${ROOT_DIR}/bin/poseidonos-cli array create -a POSArray -b uram0 -d unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3
sudo ${ROOT_DIR}/bin/poseidonos-cli array mount -a POSArray

for i in `seq 1 $SUBSYSTEM_NUM`
do
    sudo ${ROOT_DIR}/lib/spdk/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem$i -a -s POS0000000000000$i -d POS_VOL_$i
    sudo ${ROOT_DIR}/lib/spdk/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem$i -t TCP -a ${ip} -s ${port}
done

for i in `seq 1 $NR_VOLUME`
do
    volIndex=`expr $i - 1`
    sudo ${ROOT_DIR}/bin/poseidonos-cli volume create -v ${volname}$volIndex --size ${volume_size} --maxiops 0 --maxbw 0 -a POSArray
done

sudo ${ROOT_DIR}/lib/spdk/scripts/rpc.py nvmf_get_subsystems

sudo ${ROOT_DIR}/bin/poseidonos-cli wbt mfs_dump_files_list --array POSArray --output file_list.txt --volume 0
fdList=`sudo cat ./file_list.txt | jq '.filesInfoList[] | select(.fileName | test("VSAMap")?) | .fd'`
sudo rm -rf ./file_list.txt

for i in $fdList;
do
    sudo ${ROOT_DIR}/bin/poseidonos-cli wbt mfs_setup_meta_fio_test --name ${volname} --size $i
done

for i in `seq 1 $NR_VOLUME`
do
    volIndex=`expr $i - 1`
    sudo ${ROOT_DIR}/bin/poseidonos-cli volume mount -v ${volname}$volIndex -a POSArray

    # sudo nvme connect -t tcp -n nqn.2019-04.pos:subsystem$i -a ${ip} -s 1158
done

sleep 3

sudo ./fio_meta_bench.py -t TCP -i ${ip} -p ${port} -n ${NR_VOLUME}

for i in `seq 1 $NR_VOLUME`
do
    volIndex=`expr $i - 1`
    sudo ${ROOT_DIR}/bin/poseidonos-cli volume unmount -v ${volname}$volIndex -a POSArray --force

    # sudo nvme disconnect -n nqn.2019-04.pos:subsystem$i
done

if [ $wbt -eq 1 ]; then
    sudo ${ROOT_DIR}/bin/poseidonos-cli wbt flush_gcov
fi

sudo ${ROOT_DIR}/bin/poseidonos-cli array unmount -a POSArray --force
sudo ${ROOT_DIR}/bin/poseidonos-cli system stop --force

exit 0

#!/bin/bash

rootdir=$(readlink -f $(dirname $0))/../../..
spdkdir=${rootdir}/lib/spdk
createibof=$1
# [normal, degraded]
arraymode=$2
totalsize=$3
numofvol=$4
numofsubsystem=$numofvol
ip=$5
limitspeed=0


PORT_NUM=1

sizepervol=`expr $totalsize / $numofvol`
MBtoB=$((1024*1024))
GBtoB=$((1024*${MBtoB}))
sizepervol=`expr $sizepervol \* $GBtoB`

## 
sudo $spdkdir/scripts/rpc.py nvmf_create_transport -t TCP -b 64 -n 4096
sudo $spdkdir/scripts/rpc.py bdev_malloc_create -b uram0 1024 512
sudo $rootdir/bin/cli request scan_dev
# VM on VxRail server has only limited nvme drives

create_array=0
degradedmode=0
if [ ! -z $createibof ] && [ $createibof == "create" ]; then
    create_array=1
fi

if [ ! -z $arraymode ] && [ $arraymode == "degraded" ]; then
    degradedmode=1
fi

for ((i=1;i<=$numofsubsystem;i++))
do
    sudo $spdkdir/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.ibof:subsystem${i} -a -s IBOF0000000000000${i} -d IBOF_VOLUME_EXTENTION
    port=`expr $i % $PORT_NUM + 1158`
    sudo $spdkdir/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.ibof:subsystem${i} -t tcp -a ${ip} -s ${port}
done

if [ $create_array -eq 1 ]; then
    sudo $rootdir/bin/cli array reset
    if [ $degradedmode -eq 1 ]; then
        sudo $rootdir/bin/cli request create_array -b uram0 -d unvme-ns-0,unvme-ns-1,unvme-ns-2
    else
        sudo $rootdir/bin/cli request create_array -b uram0 -d unvme-ns-0,unvme-ns-1,unvme-ns-2 -s unvme-ns-3
    fi
    
    sudo $rootdir/bin/cli request mount_ibofos
    
    if [ $degradedmode -eq 1 ]; then
        sudo $root_dir/script/detach_device.sh unvme-ns-0 1
    fi
else
    sudo $rootdir/bin/cli request mount_ibofos
fi

for ((i=1;i<=$numofvol;i++))
do
    if [ ${create_array} -eq 1 ]; then
        sudo $rootdir/bin/cli request create_vol --name vol$i --size $sizepervol --maxbw ${limitspeed}
    fi
    sudo $rootdir/bin/cli request mount_vol --name vol$i
done

sudo $spdkdir/scripts/rpc.py nvmf_get_subsystems
sudo $rootdir/bin/cli request set_log_level --level info

#for ((i=1;i<=$numofvol;i++))
#do
#sudo $rootdir/lib/spdk/scripts/rpc.py bdev_set_qos_limit vol$i --rw_mbytes_per_sec $limitspeed
#done
#sudo ./fio_bench.py -t tcp -i 10.100.11.15 -p 1158 -n 1

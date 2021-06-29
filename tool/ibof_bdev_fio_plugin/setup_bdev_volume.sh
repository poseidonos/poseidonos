#!/bin/bash
# Note : increase NR_VOLUME will make multiple volumes and namespace
TEST_ON_METAFS=1
NR_VOLUME=1
VOLUME_SIZE=2147483648
#VirtualMachine?
IS_VM=false
SPDK_DIR=../../lib/spdk
BIN_PATH=../../bin
ibofos_bringup(){
        clean=$1
        sudo $SPDK_DIR/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem1 -a -s POS00000000000001 -d POS_VOLUME_EXTENTION

        sudo $SPDK_DIR/scripts/rpc.py bdev_malloc_create -b uram0 1024 512

        sudo $BIN_PATH/cli request scan_dev
        if [ "$clean" -eq 1 ]; then
                sudo $BIN_PATH/cli array reset
                sudo $BIN_PATH/cli request create_array -b uram0 -d unvme-ns-0,unvme-ns-1,unvme-ns-2  -s unvme-ns-3
				sudo $BIN_PATH/cli request mount_ibofos
                # sudo $BIN_PATH/cli request mount_arr -ft 1 -b 1 uram0 -d 3 mock1 mock2 mock3 -s 1 mock4
                for i in `seq 1 $NR_VOLUME`
                do
                        sudo $BIN_PATH/cli request create_vol --name vol$i --size ${VOLUME_SIZE}  --maxiops 0 --maxbw 0
                        sudo $BIN_PATH/cli request mount_vol --name vol$i
                done
                sudo $BIN_PATH/cli file cond_signal.json #condition variable can be signaled with this API
        else

                echo "ibofos dirty bringup"
                #TODO : need to backup uram before load_array
                sudo $BIN_PATH/cli request mount_ibofos
                for i in `seq 1 $NR_VOLUME`
                do
                    sudo $BIN_PATH/cli request mount_vol --name vol$i
                done
       fi

}

if [ $# -eq 1 ]; then
        if [ "$1" -eq 1 ]; then
                echo "ibofos clean bringup"
                ibofos_bringup 1
        else
                echo "ibofos dirty bringup"
                ibofos_bringup 0
        fi
else
        echo "ibofos clean bringup"
        ibofos_bringup 1
fi

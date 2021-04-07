#!/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/../../../

TRANSPORT=TCP
TARGET_IP=127.0.0.1
PORT_COUNT=1
SUBSYSTEM_COUNT=3
VOLUME_COUNT=3
CLEAN_BRINGUP=1
TEST_COUNT=5

check_stopped()
{
    while [ `pgrep ibofos -c` -ne 0 ]
    do
        sleep 0.5
    done
}

pkill -9 ibofos
check_stopped
${ROOT_DIR}/test/regression/start_ibofos.sh

sudo $ROOT_DIR/test/system/io_path/setup_ibofos_nvmf_volume.sh -c $CLEAN_BRINGUP -t $TRANSPORT -a $TARGET_IP -s $SUBSYSTEM_COUNT -v $VOLUME_COUNT

sudo $ROOT_DIR/test/system/io_path/nvmf_initiator_nvme_cli.sh -v $VOLUME_COUNT -r $TEST_COUNT -t $TRANSPORT -a $TARGET_IP

sudo $ROOT_DIR/test/system/nvmf/initiator/fio_full_bench.py --iodepth 128 --io_size 10m --file_num $VOLUME_COUNT --ramp_time 0 --run_time 120 --time_based 0 --bs 512,4K,128K,512-128K --trtype $TRANSPORT --traddr $TARGET_IP

${ROOT_DIR}/bin/cli wbt flush_gcov

pkill -9 ibofos
check_stopped

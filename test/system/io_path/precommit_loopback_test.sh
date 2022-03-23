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
    while [ `pgrep poseidonos -c` -ne 0 ]
    do
        sleep 0.5
    done
}

while getopts i:t:s:v:c:n: ARG ; do
    case $ARG in
        i )
            TARGET_IP=$OPTARG
            ;;
        t )
            TRANSPORT=$OPTARG
            ;;
        s )
            SUBSYSTEM_COUNT=$OPTARG
            ;;
        v )
            VOLUME_COUNT=$OPTARG
            ;;
        c )
            CLEAN_BRINGUP=$OPTARG
            ;;
        n )
            TEST_COUNT=$OPTARG
            ;;
    esac
done

pkill -9 poseidonos
check_stopped
${ROOT_DIR}/test/regression/start_poseidonos.sh

sudo $ROOT_DIR/test/system/io_path/setup_ibofos_nvmf_volume.sh -c $CLEAN_BRINGUP -t $TRANSPORT -a $TARGET_IP -s $SUBSYSTEM_COUNT -v $VOLUME_COUNT

sudo $ROOT_DIR/test/system/io_path/nvmf_initiator_nvme_cli.sh -v $VOLUME_COUNT -r $TEST_COUNT -t $TRANSPORT -a $TARGET_IP

if [ $? -ne 0 ];then
    echo "Precommit Failed : nvme connection test"
    pkill -9 poseidonos
    check_stopped
    exit 1
fi

sleep 10
sudo $ROOT_DIR/bin/poseidonos-cli telemetry start

sudo $ROOT_DIR/test/system/nvmf/initiator/fio_full_bench.py --iodepth 128 --io_size 10m --file_num $VOLUME_COUNT --ramp_time 0 --run_time 120 --time_based 0 --bs 512,4K,128K,512-128K --trtype $TRANSPORT --traddr $TARGET_IP

${ROOT_DIR}/bin/poseidonos-cli wbt flush_gcov

pkill -9 poseidonos
check_stopped

#!/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/../../

TRANSPORT=TCP
TARGET_IP=127.0.0.1
PORT_COUNT=1
SUBSYSTEM_COUNT=3
VOLUME_COUNT=3
CLEAN_BRINGUP=1
TEST_COUNT=5
target_fabric_ip=0

check_stopped()
{
    while [ `pgrep poseidonos -c` -ne 0 ]
    do
        echo "Waiting for POS stopped"
        sleep 0.5
    done
}

print_result()
{
    local result=$1
    local expectedResult=$2

    if [ $expectedResult -eq 0 ]; then
        echo -e "\033[1;34m${date} [result] ${result} \033[0m" 1>&2;
    else
        echo -e "\033[1;41m${date} [TC failed] ${result} \033[0m" 1>&2;
    fi
}

EXPECT_PASS()
{
    local name=$1
    local result=$2

    if [ $result -eq 0 ]; then
        print_result "\"${name}\" passed as expected" 0
    else
        print_result "\"${name}\" failed as unexpected" 1
        pkill -9 poseidonos
        exit 1
    fi
}

io_test()
{
    ${ROOT_DIR}/test/regression/start_poseidonos.sh
    EXPECT_PASS "start_poseidonos.sh" $?

    sleep 10
    ${ROOT_DIR}/bin/poseidonos-cli telemetry start

    sudo $ROOT_DIR/test/system/io_path/setup_ibofos_nvmf_volume.sh -c $CLEAN_BRINGUP -t $TRANSPORT -a $TARGET_IP -s $SUBSYSTEM_COUNT -v $VOLUME_COUNT
    EXPECT_PASS "setup_ibofos_nvmf_volume.sh" $?

    sudo $ROOT_DIR/test/system/io_path/nvmf_initiator_nvme_cli.sh -v $VOLUME_COUNT -r $TEST_COUNT -t $TRANSPORT -a $TARGET_IP
    EXPECT_PASS "nvmf_initiator_nvme_cli.sh" $?

    sudo $ROOT_DIR/test/system/nvmf/initiator/fio_full_bench.py --iodepth 128 --io_size 10m --file_num $VOLUME_COUNT --ramp_time 0 --run_time 120 --time_based 0 --bs 512,4K,128K,512-128K --trtype $TRANSPORT --traddr $TARGET_IP

}

print_help()
{
    echo "Multi Volume IO Test"
    echo "Usage : ./multi_volume_io_ci_test.sh -f [target_fabric_ip]"
}

while getopts ":h:f:" opt
do
    case "$opt" in
        f) target_fabric_ip="$OPTARG"
            ;;
        h) print_help
            ;;
        ?) exit 2
            ;;
    esac
done


pkill -9 poseidonos
check_stopped
io_test
pkill -9 poseidonos
check_stopped

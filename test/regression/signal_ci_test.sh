#!/bin/bash

shutdown_mode="normal"
source tc_lib_ci.sh

############################
# tc_npor_2
# [create] vol1 -> [mount] vol1 -> [write] vol1 -> NPOR -> [mount] vol1 -> [verify] vol1 -> [unmount] vol1 -> [delete] vol1 -> NPOR
############################
tc_npor_2()
{
    tcName="tc_npor_2"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 -> [mount] vol1 -> [write] vol1 -> NPOR -> [mount] vol1 -> [verify] vol1 -> [unmount] vol1 -> [delete] vol1 -> NPOR"

    bringup_pos create
    EXPECT_PASS "bringup_pos" $?

    create_and_check '1' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    write_data '1'
    EXPECT_PASS "write_data" $?

    npor_and_check_volumes complex
    EXPECT_PASS "npor_and_check_volumes" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    verify_data '1'
    EXPECT_PASS "verify_data" $?

    unmount_and_check '1'
    EXPECT_PASS "unmount_and_check" $?

    delete_and_check '1'
    EXPECT_PASS "delete_and_check" $?

    npor_and_check_volumes complex
    EXPECT_PASS "npor_and_check_volumes" $?

    end_tc "${tcName}"
    graceful_shutdown complex
}

############################
# tc_npor_3
# { [create] vol1 -> ([mount] vol1 -> [write] vol1 -> [unmount] vol1 -> [NPOR] } x 50
############################
tc_npor_3()
{
    get_test_iteration_cnt 5;
    tcTestCount=$?

    tcName="tc_npor_3"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: { [create] vol1 -> ([mount] vol1 -> [write] vol1 -> [unmount] vol1 -> [NPOR] } x ${tcTestCount}"

    bringup_pos create
    EXPECT_PASS "bringup" $?

    create_and_check '1' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    for fidx in `seq 1 ${tcTestCount}`
    do
        print_notice "TC=${tcName} : All test count=${fidx}, total=${tcTestCount}"

        mount_and_check '1'
        EXPECT_PASS "mount_and_check" $?

        write_data '1'
        EXPECT_PASS "write_data" $?

        unmount_and_check '1'
        EXPECT_PASS "unmount_and_check" $?
        
        npor_and_check_volumes complex
        EXPECT_PASS "npor_and_check_volumes" $?

    done

    end_tc "${tcName}"
    graceful_shutdown complex
}

############################
# tc_npor_4
# complicated
############################
tc_npor_4()
{
    tcName="tc_npor_4"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: complicated"

    bringup_pos create
    EXPECT_PASS "bringup" $?

    create_and_check '1' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    create_and_check '2' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    delete_and_check '1'
    EXPECT_PASS "delete_and_check" $?

    npor_and_check_volumes complex
    EXPECT_PASS "npor_and_check_volumes" $?

    create_and_check '3' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    delete_and_check '2'
    EXPECT_PASS "delete_and_check" $?

    delete_and_check '3'
    EXPECT_PASS "delete_and_check" $?

    npor_and_check_volumes complex
    EXPECT_PASS "npor_and_check_volumes" $?

    create_and_check '4' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    create_and_check '5' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    delete_and_check '4'
    EXPECT_PASS "delete_and_check" $?

    npor_and_check_volumes complex
    EXPECT_PASS "npor_and_check_volumes" $?

    create_and_check '6' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    delete_and_check '6'
    EXPECT_PASS "delete_and_check" $?

    end_tc "${tcName}"
    abrupt_shutdown
}

############################
# tc array
############################
run()
{
    ####################################
    # add tc name here
    ####################################
    if [ "$test_mode" == "precommit" ];
    then
        max_test_iteration=1
    fi

   
    tc_array=(
            tc_npor_2 tc_npor_3 tc_npor_4
        )

    tcTotalCount=${#tc_array[@]}

    ####################################
    # setting env.
    ####################################
    # 0: loop-back(vm), 1: nvmeof(pm), 2: echo
    exec_mode=0

    # # of subsystems
    support_max_subsystem=2

    # protocol
    trtype="tcp"

    # # target ip
    # target_ip=127.0.0.1
    target_ip=`ifconfig | awk '$1 == "inet" {gsub(/\/.*$/, "", $2); print $2}' | sed -n '1,1p'`
    target_fabric_ip=`ifconfig | awk '$1 == "inet" {gsub(/\/.*$/, "", $2); print $2}' | sed -n '2,1p'`

    # root dir: initiator
    rootInit=$(readlink -f $(dirname $0))/../..

    # root dir: target
    rootTarget=$(readlink -f $(dirname $0))/../..
    ####################################

    update_config

    local tcList=""
    for fidx in "${!tc_array[@]}"
    do
        ${tc_array[fidx]}
    done

    print_notice "All TCs (${tcTotalCount} TCs) have passed.\n"
}

get_test_iteration_cnt()
{
    if [[ $1 -gt ${max_test_iteration} ]]; then
        return ${max_test_iteration}
    else
        return $1
    fi
}

isVm=0
max_test_iteration=100
test_mode=""

while getopts "f:v:p:s:" opt
do
    case "$opt" in
        f) target_fabric_ip="$OPTARG"
            ;;
        v) isVm="$OPTARG"
            ;;
        p) test_mode="precommit"
            ;;
        s) shutdown_mode="$OPTARG"
            ;;
        ?) exit 2
            ;;
    esac
done

network_module_check
run;

exit 0


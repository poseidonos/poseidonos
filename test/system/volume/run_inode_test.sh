#!/bin/bash

source tc_lib.sh

############################
# tc_inode_0
############################
tc_inode_0()
{
    bringup_ibofos create
    EXPECT_PASS "bringup" $?

    tcName="tc_inode_0"
    show_tc_info "${tcName}"
    start_tc "${tcName}"

    create_and_check '2' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    create_and_check '1' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    delete_and_check '2'
    EXPECT_PASS "delete_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    write_data '1'
    EXPECT_PASS "write_data" $?

    unmount_and_check '1'
    EXPECT_PASS "unmount_and_check" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    verify_data '1'
    EXPECT_PASS "verify_data" $?

    end_tc "${tcName}"
    abrupt_shutdown
}

############################
# tc_inode_1
############################
tc_inode_1()
{
    bringup_ibofos create
    EXPECT_PASS "bringup" $?

    tcName="tc_inode_1"
    show_tc_info "${tcName}"
    start_tc "${tcName}"

    for idx in {1..1000}
    do
        print_info "test #${idx}"

        create_and_check '1' 2147483648 0 0
        EXPECT_PASS "create_and_check" $?

        delete_and_check '1'
        EXPECT_PASS "delete_and_check" $?
    done

    end_tc "${tcName}"
    #abrupt_shutdown
}

############################
# tc_inode_2
############################
tc_inode_2()
{
    cd ${rootInit}/script
    ./setup_env.sh

    bringup_ibofos create
    EXPECT_PASS "bringup" $?

    tcName="tc_inode_2"
    show_tc_info "${tcName}"
    start_tc "${tcName}"

    for idx in {1..1000}
    do
        print_info "test #${idx}"

        npor_and_check_volumes
        EXPECT_PASS "npor_and_check_volumes" $?
    done

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
    tc_array=(
                tc_inode_0
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

    # target ip
    target_ip=`ifconfig | awk '$1 == "inet" {gsub(/\/.*$/, "", $2); print $2}' | sed -n '1,1p'`
    target_fabric_ip=`ifconfig | awk '$1 == "inet" {gsub(/\/.*$/, "", $2); print $2}' | sed -n '2,1p'`
    #target_ip="10.1.1.10"
    #target_fabric_ip="172.16.1.1"

    # root dir: initiator
    rootInit=$(readlink -f $(dirname $0))/../../..

    # root dir: target
    rootTarget=$(readlink -f $(dirname $0))/../../..
    ####################################

    update_config

    local tcList=""

    for fidx in "${!tc_array[@]}"
    do
        ${tc_array[fidx]}
        tcList+="   $((fidx+1)): ${tc_array[fidx]}\n"
    done

    print_notice "All TCs (${tcTotalCount} TCs) have passed.\n${tcList}"
}

run;

exit 0

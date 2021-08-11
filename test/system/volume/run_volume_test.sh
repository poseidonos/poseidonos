#!/bin/bash

source tc_lib.sh

############################
# tc_vol_0
# do nothing
############################
tc_vol_0()
{
    tcName="tc_vol_0"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: do nothing"

    bringup_pos create
    EXPECT_PASS "bringup" $?

    end_tc "${tcName}"
    abrupt_shutdown
}

############################
# tc_vol_1
# [create] vol1 → [delete] vol1
############################
tc_vol_1()
{
    tcName="tc_vol_1"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 → [delete] vol1"

    bringup_pos create
    EXPECT_PASS "bringup" $?

    create_and_check '1' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    delete_and_check '1'
    EXPECT_PASS "delete_and_check" $?

    end_tc "${tcName}"
    abrupt_shutdown
}

############################
# tc_vol_2
# [create] vol1 → [mount] vol1 → [unmount] vol1 → [delete] vol1
############################
tc_vol_2()
{
    tcName="tc_vol_2"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 → [mount] vol1 → [unmount] vol1 → [delete] vol1"

    bringup_pos create
    EXPECT_PASS "bringup" $?

    create_and_check '1' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    unmount_and_check '1'
    EXPECT_PASS "unmount_and_check" $?

    delete_and_check '1'
    EXPECT_PASS "delete_and_check" $?

    end_tc "${tcName}"
    abrupt_shutdown
}

############################
# tc_vol_3
# [create] vol1 → [mount] vol1 → [io] vol1 → NPOR → [delete] vol1
############################
tc_vol_3()
{
    tcName="tc_vol_3"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 → [mount] vol1 → [io] vol1 → NPOR → [delete] vol1"

    bringup_pos create
    EXPECT_PASS "bringup" $?

    create_and_check '1' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    write_and_verify '1'
    EXPECT_PASS "write_and_verify" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    delete_and_check '1'
    EXPECT_PASS "delete_and_check" $?

    end_tc "${tcName}"
    abrupt_shutdown
}

############################
# tc_vol_4
# [create] vol1 → [mount] vol1 → [create] vol2 → [mount] vol2 → [unmount] vol2 → [delete] vol2 
############################
tc_vol_4()
{
    tcName="tc_vol_4"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 → [mount] vol1 → [create] vol2 → [mount] vol2 → [unmount] vol2 → [delete] vol2"

    bringup_pos create
    EXPECT_PASS "bringup" $?

    create_and_check '1' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    create_and_check '2' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '2'
    EXPECT_PASS "mount_and_check" $?

    unmount_and_check '2'
    EXPECT_PASS "unmount_and_check" $?

    delete_and_check '2'
    EXPECT_PASS "delete_and_check" $?

    end_tc "${tcName}"
    abrupt_shutdown
}

############################
# tc_vol_5
# [create] vol1 → [mount] vol1 → [create] vol2 → [delete] vol2 → NPOR
############################
tc_vol_5()
{
    tcName="tc_vol_5"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 → [mount] vol1 → [create] vol2 → [delete] vol2 → NPOR"

    bringup_pos create
    EXPECT_PASS "bringup" $?

    create_and_check '1' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    create_and_check '2' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    delete_and_check '2'
    EXPECT_PASS "delete_and_check" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    end_tc "${tcName}"
    abrupt_shutdown
}

############################
# tc_npor_0
# [create] vol1 → [mount] vol1 → [write] vol1 → NPOR → [mount] vol1 → [verify] vol1
############################
tc_npor_0()
{
    tcName="tc_npor_0"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: vol1 → [mount] vol1 → [write] vol1 → NPOR → [mount] vol1 → [verify] vol1"

    bringup_pos create
    EXPECT_PASS "bringup" $?

    create_and_check '1' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    write_data '1'
    EXPECT_PASS "write_data" $?

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
# tc_npor_1
# [create] vol1 → [mount] vol1 → [create] vol2 → [mount] vol2 → [write] vol1 → [write] vol2 → NPOR → [mount] vol1 → [verify] vol1 → [mount] vol2 → [verify] vol2
############################
tc_npor_1()
{
    tcName="tc_npor_1"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 → [mount] vol1 → [create] vol2 → [mount] vol2 → [write] vol1 → [write] vol2 → NPOR → [mount] vol1 → [verify] vol1 → [mount] vol2 → [verify] vol2"

    bringup_pos create
    EXPECT_PASS "bringup" $?

    create_and_check '1' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    create_and_check '2' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '2'
    EXPECT_PASS "mount_and_check" $?

    write_data '1'
    EXPECT_PASS "write_data" $?

    write_data '2'
    EXPECT_PASS "write_data" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    verify_data '1'
    EXPECT_PASS "verify_data" $?

    mount_and_check '2'
    EXPECT_PASS "mount_and_check" $?

    verify_data '2'
    EXPECT_PASS "verify_data" $?

    end_tc "${tcName}"
    abrupt_shutdown
}

############################
# tc_npor_2
# [create] vol1 → [mount] vol1 → [write] vol1 → NPOR → [mount] vol1 → [verify] vol1 → [unmount] vol1 → [delete] vol1 → NPOR
############################
tc_npor_2()
{
    tcName="tc_npor_2"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 → [mount] vol1 → [write] vol1 → NPOR → [mount] vol1 → [verify] vol1 → [unmount] vol1 → [delete] vol1 → NPOR"

    bringup_pos create
    EXPECT_PASS "bringup" $?

    create_and_check '1' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    write_data '1'
    EXPECT_PASS "write_data" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    verify_data '1'
    EXPECT_PASS "verify_data" $?

    unmount_and_check '1'
    EXPECT_PASS "unmount_and_check" $?

    delete_and_check '1'
    EXPECT_PASS "delete_and_check" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    end_tc "${tcName}"
    abrupt_shutdown
}

############################
# tc_npor_3
# { [create] vol1 -> ([mount] vol1 -> [write] vol1 -> [unmount] vol1 -> [NPOR] } x multiple
############################
tc_npor_3()
{
    tcTestCount=10
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
        
        npor_and_check_volumes
        EXPECT_PASS "npor_and_check_volumes" $?

    done

    end_tc "${tcName}"
    graceful_shutdown
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

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    create_and_check '3' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    delete_and_check '2'
    EXPECT_PASS "delete_and_check" $?

    delete_and_check '3'
    EXPECT_PASS "delete_and_check" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    create_and_check '4' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    create_and_check '5' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    delete_and_check '4'
    EXPECT_PASS "delete_and_check" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    create_and_check '6' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    delete_and_check '6'
    EXPECT_PASS "delete_and_check" $?

    end_tc "${tcName}"
    abrupt_shutdown
}

############################
# tc_spor_0
# [create] vol1 → [mount] vol1 → [write] vol1 → SPOR → [mount] vol1 → [verify] vol1
############################
tc_spor_0()
{
    tcName="tc_spor_0"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "[create] vol1 → [mount] vol1 → [write] vol1 → SPOR → [mount] vol1 → [verify] vol1"

    bringup_pos create
    EXPECT_PASS "bringup" $?

    create_and_check '1' 2GB 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    write_data '1'
    EXPECT_PASS "write_data" $?

    spor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    verify_data '1'
    EXPECT_PASS "verify_data" $?

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
                tc_vol_0 tc_vol_1 tc_vol_2 tc_vol_3 tc_vol_4 tc_vol_5
                tc_npor_0 tc_npor_1 tc_npor_2 tc_npor_3 tc_npor_4
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

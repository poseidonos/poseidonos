#!/bin/bash
#source tc_lib.sh
#**************************************************************************
# VARIABLE INITIALIZATION
#**************************************************************************
logfile="/var/log/pos/qos_test.log"
INITIATOR_ROOT_DIR=$(readlink -f $(dirname $0))/../../../

INITIATOR_SPDK_DIR=$INITIATOR_ROOT_DIR/lib/spdk
CONFIG_FILE=/etc/pos/pos.conf

network_config_file=test/system/network/network_config.sh

# Note: In case of tcp transport, network io irq can be manually controlled for better performance by changing SET_IRQ_AFFINITY=TRUE with given TARGET_NIC and NET_IRQ_CPULIST

CLEAN_BRINGUP=1
detach_dev1="unvme-ns-0"
detach_dev2="unvme-ns-1"
spare_dev1="unvme-ns-3"
spare_dev2="unvme-ns-4"


VOLUME_SIZE=2147483648
NUM_REACTORS=31
DEFAULT_NR_VOLUME=31
DEFAULT_SUBSYSTEM=31
DEFAULT_PORT=1158
TARGET_ROOT_DIR=$(readlink -f $(dirname $0))/../../..
TARGET_SPDK_DIR=$TARGET_ROOT_DIR/lib/spdk


#**************************************************************************
# TEST SETUP INFORMATION
#**************************************************************************
show_test_setup_info(){
    echo -e "======================================================="
    if [  ${EXEC_MODE} == 1 ]; then
        echo -e "==  Loop Back Mode"
    else
        echo -e "==  Initiator Target Setup"
    fi
    echo -e "==  TARGET FABRIC IP: ${TARGET_IP}"
    echo -e "==  TRANSPORT: ${TRANSPORT}"
    echo -e "==  PORT: ${PORT}"
    echo -e "==  TARGET SYSTEM IP: ${TARGET_SYSTEM_IP}"
    echo -e "==  TARGET ROOT DIRECTORY: ${TARGET_ROOT_DIR}"
    echo -e "==  SUBSYSTEM COUNT: ${SUBSYSTEM}"
    echo -e "==  VOLUME COUNT: ${NR_VOLUME}"
    echo -e "======================================================="
}

#**************************************************************************
# CONSOLE MESSAGES
#**************************************************************************
log_normal(){
    echo -e $GREEN_COLOR$1$RESET_COLOR
}

log_error(){
    echo -e $RED_COLOR$1$RESET_COLOR
}

#**************************************************************************
# TC_LIB FUNCTIONS
#**************************************************************************
print_notice()
{
    echo -e "\033[1;36m${date} [notice] $@ \033[0m" 1>&2;
}

print_info()
{
    echo -e "\033[1;34m${date} [info] $@ \033[0m" 1>&2;
}

print_result()
{
    local result=$1
    local expectedResult=$2

    if [ $expectedResult -eq 0 ];then
        echo -e "\033[1;34m${date} [result] ${result} \033[0m" 1>&2;
    else
        echo -e "\033[1;41m${date} [TC failed] ${result} \033[0m" 1>&2;
    fi
}

start_tc()
{
    local tcName=$1

    echo -e "\033[1;35m${date} [TC start] $@ \033[0m" 1>&2;
    let tcCount=$tcCount+1
}

end_tc()
{
    local tcName=$1

    echo -e "\033[1;35m${date} [TC end] $@, passed ${tcCount} / ${tcTotalCount} \033[0m" 1>&2;
    echo -e ""
}

show_tc_info()
{
    local tcName=$1
    print_notice "Information for \"${tcName}\""
}


abrupt_shutdown()
{
    local withBackup=$1

    print_info "Shutting down suddenly in few seconds..."

    kill_pos

    if [ "${withBackup}" != "" ]; then
        texecc $TARGET_ROOT_DIR/script/backup_latest_hugepages_for_uram.sh
        sleep 3
    fi

    for i in `seq 1 ${support_max_subsystem}`
    do
        disconnect_nvmf_controllers ${i}
    done
    print_info "Shutdown has been completed!"
}


EXPECT_PASS()
{
    local name=$1
    local result=$2

    if [ $result -eq 0 ];then
        print_result "\"${name}\" passed as expected" 0
    else
        print_result "\"${name}\" failed as unexpected" 1
        abrupt_shutdown
        exit 1
    fi
}

EXPECT_FAIL()
{
    local name=$1
    local result=$2

    if [ $result -ne 0 ];then
        print_result "\"${name}\" failed as expected" 0
    else
        print_result "\"${name}\" passed as unexpected" 1
        abrupt_shutdown
        exit 1
    fi
}

add_spare()
{
    spare_dev=$1
    print_info "Add Spare device ${spare_dev}"
    texecc $TARGET_ROOT_DIR/bin/cli array add --name POSArray --spare ${spare_dev}
}

pci_rescan()
{
    echo "Rescan Pci Devices"
    texecc echo 1 > /sys/bus/pci/rescan
}

detach_device()
{
    detach_dev=$1
    texecc ${TARGET_ROOT_DIR}/test/script/detach_device.sh ${detach_dev} 1
    sleep 0.1
    print_info "${detach_dev} is detected"
}


#**************************************************************************
# SETUP_SPDK
#**************************************************************************
setup_spdk(){
    texecc $TARGET_SPDK_DIR/script/setup.sh reset
    texecc sleep 10s
}

#**************************************************************************
# RESET SPDK
#**************************************************************************
reset_spdk(){
    texecc $TARGET_ROOT_DIR/script/setup_env.sh
    texecc sleep 10s
}

#**************************************************************************
# EXIT POS
#**************************************************************************
exit_pos(){
    texecc $TARGET_ROOT_DIR/bin/cli array unmount --name POSArray
    texecc sleep 10
    if [ 1 == ${?} ]; then
        kill_pos
        return
    fi
    echo "Unmount is complete" >> result

    texecc $TARGET_ROOT_DIR/bin/cli system exit
    texecc ps -C poseidonos > /dev/null >> ${logfile}

    n=1
    while [[ ${?} == 0 ]]
    do
        if [ $n -eq 30 ]; then
            kill_pos
            return
        fi 
        texecc sleep 10
        n=$(( n+1 ))
        print_info "Waiting for POS to exit ($n of 30)"
        texecc ps -C poseidonos > /dev/null >> ${logfile}
    done
    print_info "POS Instance Exited"
}

#**************************************************************************
# WAITING FOR REBUILD COMPLETION
#**************************************************************************
waiting_for_rebuild_complete(){
    ret=1
    n=1
    while [ $n -le 360 ]
    do
        state=$($TARGET_ROOT_DIR/bin/cli array info --name POSArray --json | jq '.Response.result.data.state')
        echo "current state : "$state
        if [ $state == "\"NORMAL\"" ]; then
            print_info "Rebuild Completed"
            ret=0
            break;
        else
            texecc sleep 10
            rebuild_progress=$($TARGET_ROOT_DIR/bin/cli array info --name POSArray --json | jq '.Response.result.data.rebuildingProgress')
            info "Rebuilding Progress [${rebuild_progress}]"
            print_info "Waiting for Rebuild to Complete ($n of 360)"
        fi
        n=$(( n+1 ))
    done
    return $ret
}

###################################################
# Execution in Target Server
###################################################
texecc(){
     case ${EXEC_MODE} in
     0) # default test
         echo "[target]" $@;
         sshpass -p $TARGET_PWD ssh -tt -o StrictHostKeyChecking=no $TARGET_USERNAME@$TARGET_SYSTEM_IP "cd ${TARGET_ROOT_DIR}; sudo $@"
         ;;
     1) # echo command
         echo "[target]" $@;
         cd ${TARGET_ROOT_DIR};
         sudo $@
         ;;
     esac
}


#**************************************************************************
# Check Environment
#**************************************************************************
check_env(){
    if [ ! -f /usr/bin/sshpass ]; then
        sudo apt install -y sshpass &> /dev/null
        if [ ! -f /usr/bin/sshpass ]; then
            exit 2;
        fi
    fi
}
#**************************************************************************
# Setup Pre-requisites
#**************************************************************************
setup_prerequisite(){
    chmod +x $INITIATOR_ROOT_DIR/*.sh
    chmod +x $INITIATOR_ROOT_DIR/$network_config_file

    texecc chmod +x script*.sh >> ${logfile}
    texecc chmod +x ${network_config_file} >> ${logfile}

    if [ ${echo_slient} -eq 1 ] ; then
        rm -rf ${logfile};
        touch ${logfile};
    fi

    texecc ls /sys/class/infiniband/*/device/net >> ${logfile}
    ls /sys/class/infiniband/*/device/net >> ${logfile}

    if [ ${TRANSPORT} == "rdma" || ${TRANSPORT} == "RDMA" ]; then
        echo -n "RDMA configuration for server..."
        texecc ./${network_config_file} server >> ${logfile}
        wait
        echo "Done"

        echo -n "RDMA configuration for client..."
        $INITIATOR_ROOT_DIR/${network_config_file} client >> ${logfile}
        wait
        echo "Done"
    fi
}

#**************************************************************************
#Kill POS
#**************************************************************************
kill_pos(){
    texecc $TARGET_ROOT_DIR/test/script/kill_poseidonos.sh
    echo ""
    texecc sleep 2
    texecc ps -C poseidonos > /dev/null >> ${logfile}
    echo "$?"
    while [[ ${?} == 0 ]]
    do
        echo "$?"
        texecc sleep 1s
        texecc ps -C poseidonos > /dev/null >> ${logfile}
    done
    return
    echo "Old Instance POS is killed"
}

#**************************************************************************
#Disconect the POS volumes as NVMf target devices
#**************************************************************************
disconnect_nvmf_controllers() {
    num_subsystems=$1
    echo "Disconnecting devices" >> ${logfile}
    for i in $(seq 1 $num_subsystems)
    do
        sudo nvme disconnect -n nqn.2019-04.pos:subsystem$i #>> ${logfile}
    done
}

#**************************************************************************
# Check Network Module
#**************************************************************************
network_module_check(){
    texecc $TARGET_ROOT_DIR/test/regression/network_module_check.sh >> ${logfile}
}

#**************************************************************************
# Setup test environment for QoS Test Scripts
#**************************************************************************
setup_test_environment(){

    print_info "Checking Environment"
    check_env

#    print_info "Setup Prerequisite Softwares"
#    setup_prerequisite

    print_info "Killing previos POS instance, if any"
    kill_pos 0
    print_info "Disconnecting NVMf controllers, if any"
    disconnect_nvmf_controllers $max_subsystems

    print_info "Checking Network Module"
    network_module_check

    texecc $TARGET_ROOT_DIR/script/setup_env.sh
    EXPECT_PASS "setup_environment" $?
    texecc sleep 10
}

###################################################
# START POS
###################################################
start_pos(){
    texecc $TARGET_ROOT_DIR/test/regression/start_poseidonos.sh
    EXPECT_PASS "POS OS Launch"  $?

    texecc sleep 10
    texecc $TARGET_ROOT_DIR/test/system/io_path/setup_ibofos_nvmf_volume.sh -c 1 -t $TRANSPORT -a $TARGET_IP -s $SUBSYSTEM -v $NR_VOLUME -u "unvme-ns-0,unvme-ns-1,unvme-ns-2" -p "none"
    EXPECT_PASS "setup_ibofos_nvmf_volume.sh" $?
}

###################################################
# LAUNCH FIO WITH INPUT CONFIGURATIONS
###################################################
launch_fio()
{
    if [ $# -ne 9 ];then
        echo "Insufficient  Parameters, ex. launch_fio file_num num_job io_depth bs readwrite runtime group workload run_background"
        return 1
    fi

    file_num=$1
    num_job=$2
    io_depth=$3
    bs=$4
    readwrite=$5
    runtime=$6

    group=$7
    workload=$8
    run_background=$9

    echo -e "============================================================"
    echo -e "=================  FIO CONFIGURATION  ======================"
    echo -e "============================================================"
    echo -e " VOLUME COUNT      : ${file_num}"
    echo -e " NUM OF JOBS       : ${num_job}"
    echo -e " QUEUE DEPTH       : ${io_depth}"
    echo -e " BLOCK SIZE        : ${bs}"
    echo -e " IO TYPE           : ${readwrite}"
    echo -e " RUN TIME          : ${runtime}"
    echo -e " GROUP REPORTING   : ${group}"
    echo -e " CUSTOM WORKLOAD   : ${workload}"
    echo -e " TARGET FABRICS IP : ${TARGET_IP}"
    echo -e " NVMF TRANSPORT    : ${TRANSPORT}"
    echo -e " PORT NUMBER       : ${PORT}"
    echo -e "============================================================"

    if [ $run_background -eq 1 ]; then
        print_info "FIO Running in background"
        $INITIATOR_ROOT_DIR/test/system/qos/qos_fio_bench.py --file_num="${file_num}" --numjobs="${num_job}" --iodepth="${io_depth}" --bs="${bs}" --readwrite="${readwrite}" --run_time="${runtime}" --group_reporting="${group}" --workload_type="${workload}" --traddr="$TARGET_IP" --trtype="$TRANSPORT" --port="$PORT" & >> $INITIATOR_ROOT_DIR/test/system/qos/qos_fio.log
    else
        $INITIATOR_ROOT_DIR/test/system/qos/qos_fio_bench.py --file_num="${file_num}" --numjobs="${num_job}" --iodepth="${io_depth}" --bs="${bs}" --readwrite="${readwrite}" --run_time="${runtime}" --group_reporting="${group}" --workload_type="${workload}" --traddr="$TARGET_IP" --trtype="$TRANSPORT" --port="$PORT" >> $INITIATOR_ROOT_DIR/test/system/qos/qos_fio.log
    fi
}

###################################################
# FIO: 1 VOLUMES, 31 JOB
###################################################
#**************************************************************************
# Sequential Write
#**************************************************************************
tc_1v_31job_write()
{
    fio_tc_name="Type: Perfromance, Volumes:1, Jobs:31, Details: QD(4), BS(128k), Sequential Write"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    launch_fio 1 31 4 128k write 30 1 4 0
    EXPECT_PASS "FIO Launch" $?
    bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 1 --group_report 1`
    print_info "Bandwidth $bw"
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

#**************************************************************************
# Sequential Read
#**************************************************************************
tc_1v_31job_read()
{
    fio_tc_name="Type: Perfromance, Volumes:1, Jobs:31, Details: QD(4), BS(128k), Sequential Read"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    launch_fio 1 31 4 128k read 30 1 4 0
    EXPECT_PASS "FIO Launch" $?
    bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 0 --group_report 1`
    print_info "Bandwidth $bw"
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

#**************************************************************************
# Random Write
#**************************************************************************
tc_1v_31job_randwrite()
{
    fio_tc_name="Type: Perfromance, Volumes:1, Jobs:31, Details: QD(128), BS(4k), Random Write"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    launch_fio 1 31 128 4k randwrite 30 1 4 0
    EXPECT_PASS "FIO Launch" $?
    bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 1 --group_report 1`
    print_info "Bandwidth $bw"
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

#**************************************************************************
# Random Read
#**************************************************************************
tc_1v_31job_randread()
{
    fio_tc_name="Type: Perfromance, Volumes:1, Jobs:31, Details: QD(128), BS(4k), Random Read"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    launch_fio 1 31 128 4k randread 30 1 4 0
    EXPECT_PASS "FIO Launch" $?
    bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 0 --group_report 1`
    print_info "Bandwidth $bw"
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

###################################################
# FIO: 8 VOLUMES, 3 JOB
###################################################
#**************************************************************************
# Sequential Write
#**************************************************************************
tc_8v_3job_write()
{
    fio_tc_name="Type: Perfromance, Volumes:8, Jobs:3, Details: QD(4), BS(128k), Sequential Write"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    launch_fio 8 3 4 128k write 30 1 4 0
    EXPECT_PASS "FIO Launch" $?
    bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 1 --group_report 1`
    print_info "Bandwidth $bw"
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

#**************************************************************************
# Sequential Read
#**************************************************************************
tc_8v_3job_read()
{
    fio_tc_name="Type: Perfromance, Volumes:8, Jobs:3, Details: QD(4), BS(128k), Sequential Read"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    launch_fio 8 3 4 128k read 30 1 4 0
    EXPECT_PASS "FIO Launch" $?
    bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 0 --group_report 1`
    print_info "Bandwidth $bw"
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

#**************************************************************************
# Random Write
#**************************************************************************
tc_8v_3job_randwrite()
{
    fio_tc_name="Type: Perfromance, Volumes:8, Jobs:3, Details: QD(128), BS(4k), Random Write"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    launch_fio 8 3 128 4k randwrite 30 1 4 0
    EXPECT_PASS "FIO Launch" $?
    bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 1 --group_report 1`
    print_info "Bandwidth $bw"
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

#**************************************************************************
# Random Read
#**************************************************************************
tc_8v_3job_randread()
{
    fio_tc_name="Type: Perfromance, Volumes:8, Jobs:3, Details: QD(128), BS(4k), Random Read"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    launch_fio 8 3 128 4k randread 30 1 4 0
    EXPECT_PASS "FIO Launch" $?
    bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 0 --group_report 1`
    print_info "Bandwidth $bw"
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

###################################################
# FIO: 31 VOLUMES, 1 JOB
###################################################
#**************************************************************************
# Sequential Write
#**************************************************************************
tc_31v_1job_write()
{
    fio_tc_name="Type: Perfromance, Volumes:31, Jobs:1, Details: QD(4), BS(128k), Sequential Write"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    launch_fio 31 1 4 128k write 30 1 4 0
    EXPECT_PASS "FIO Launch" $?
    bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 1 --group_report 1`
    print_info "Bandwidth $bw"
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

#**************************************************************************
# Sequential Read
#**************************************************************************
tc_31v_1job_read()
{
    fio_tc_name="Type: Perfromance, Volumes:31, Jobs:1, Details: QD(4), BS(128k), Sequential Read"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    launch_fio 31 1 4 128k read 30 1 4 0
    EXPECT_PASS "FIO Launch" $?
    bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 0 --group_report 1`
    print_info "Bandwidth $bw"
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

#**************************************************************************
# Random Write
#**************************************************************************
tc_31v_1job_randwrite()
{
    fio_tc_name="Type: Perfromance, Volumes:31, Jobs:1, Details: QD(128), BS(4k), Random Write"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    launch_fio 31 1 128 4k randwrite 30 1 4 0
    EXPECT_PASS "FIO Launch" $?
    bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 1 --group_report 1`
    print_info "Bandwidth $bw"
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

#**************************************************************************
# Random Read
#**************************************************************************
tc_31v_1job_randread()
{
    fio_tc_name="Type: Perfromance, Volumes:31, Jobs:1, Details: QD(128), BS(4k), Random Read"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    launch_fio 31 1 128 4k randread 30 1 4 0
    EXPECT_PASS "FIO Launch" $?
    bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 0 --group_report 1`
    print_info "Bandwidth $bw"
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

###################################################
# QOS WRR CONFIGURATION: FLUSH
###################################################
tc_flush_wrr()
{
    fio_tc_name="Type: EventWRR, Volumes:8, Jobs:1, Event: Flush"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    print_info "Default Event WRR settings applied"
    launch_fio 8 1 4 128k read 30 1 2 0
    EXPECT_PASS "FIO Launch" $?
    before_read_bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 0 --group_report 1`
    before_write_bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 1 --group_report 1`
    print_info "Event WRR changes, Flush (Priority:2, Weight:3)"
    texecc $TARGET_ROOT_DIR/bin/cli internal update_event_wrr --name flush --prio 2 --weight 3
    launch_fio 8 1 4 128k read 30 1 2 0
    EXPECT_PASS "FIO Launch" $?
    after_read_bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 0 --group_report 1`
    after_write_bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 1 --group_report 1`
    print_info "Before Event WRR, Read BW = $before_read_bw"
    print_info "After Event WRR change, Read BW = $after_read_bw"
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

###################################################
# QOS WRR CONFIGURATION: READ REBUILD
###################################################
tc_readwrite_rebuild_wrr()
{
    fio_tc_name="Type: EventWRR, Volumes:8, Jobs:1, Event: Read Write Rebuild"
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    launch_fio 8 3 4 128k write 30 1 4 0
    EXPECT_PASS "FIO Launch" $?
    print_info "Initiate Rebuild, detach device unvme-ns-0"
    print_info "Event WRR changes, Flush (Priority:0, Weight:1)"
    texecc $TARGET_ROOT_DIR/bin/cli internal update_event_wrr --name flush --prio 0 --weight 1
    texecc $TARGET_ROOT_DIR/bin/cli internal update_event_wrr --name fe_rebuild --prio 0 --weight 1
    texecc $TARGET_ROOT_DIR/bin/cli internal update_event_wrr --name rebuild --prio 2 --weight 3
    detach_device $detach_dev1
    add_spare $spare_dev1
    texecc sleep 5s
    launch_fio 8 1 4 128k read 30 1 2 0
    before_read_bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 0 --group_report 1`
    before_write_bw=`$INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py -v 7 -io 1 --group_report 1`
    texecc $TARGET_ROOT_DIR/bin/cli internal update_event_wrr --name rebuild --prio 0 --weight 1
    print_info "Read BW = $before_read_bw"
    print_info "Write BW = $before_write_bw"
    waiting_for_rebuild_complete
    EXPECT_PASS "Rebuild1 Complete" $?
    texecc $TARGET_ROOT_DIR/bin/cli internal reset_event_wrr
}


###################################################
# RUN FIO TEST CASES
###################################################
run_fio_tests(){
    mode=$1
    base_tc=$2
    if [ $mode == "be_qos" ]; then
        print_info "BE QOS TEST CASES"
        tc_array=(tc_1v_31job_write tc_1v_31job_read tc_1v_31job_randwrite tc_1v_31job_randread
                    tc_8v_3job_write tc_8v_3job_read tc_8v_3job_randwrite tc_8v_3job_randread
                    tc_31v_1job_write tc_31v_1job_read tc_31v_1job_randwrite tc_31v_1job_randread
                    tc_flush_wrr tc_readwrite_rebuild_wrr)
    fi
    local fio_tc_list=""
    for fidx1 in "${!tc_array[@]}"
    do
        ${tc_array[fidx1]}
        fio_tc_list="   $((fidx1+1)): ${tc_array[fidx1]}\n"
    done
    print_notice "All FIO RELATED (${base_tc} TCs) have PASSED"
    for fidx1 in "${!tc_array[@]}"
    do
        echo "    ${tc_array[fidx1]}"
    done
}



###################################################
# CODE COMPILATION
###################################################
compile_pos(){
    compile_flag=$1
    compile_option=$2
    texecc $TARGET_ROOT_DIR/configure $compile_flag
    if [ $compile_option == "pos" ]; then
        texecc make clean
	    texecc make -j$(nproc)
        echo "compilation starts"
    else
        texecc $TARGET_ROOT_DIR/script/build_ibofos.sh
    fi
}

###################################################
# CODE COMPILATION TEST CASES
###################################################
compile_with_be_qos(){
    compile_tc_name="Compile POS Code, BE QoS Enabled"
    show_tc_info "${compile_tc_name}"
    start_tc "${compile_tc_name}"
#    compile_pos --with-be-qos pos
    EXPECT_PASS "${compile_tc_name}" $?
    end_tc "${compile_tc_name}"
    start_pos
    EXPECT_PASS "Successful POS Launch & Configuration" $?
    run_fio_tests be_qos compile_with_be_qos
    EXPECT_PASS "Successful Completion of FIO test cases" $?
    texecc sleep 10
    exit_pos
}

###################################################
# CODE COMPILATION SANITY TESTS
###################################################
run_qos_test_cases(){
    echo "----------------------------------------------------------------"
    echo "Test Cases To Compile POS Code with/ without QoS"
    echo "----------------------------------------------------------------"
    compile_tc_array=(compile_with_be_qos)
    compile_total_tc=${compile_tc_array[@]}
    local compile_tc_list=""

    for fidx in "${!compile_tc_array[@]}"
    do
         ${compile_tc_array[fidx]}
         compile_tc_list+="   $((fidx+1)): ${compile_tc_array[fidx]}\n"
    done
    print_notice "All COMPILATION (${compile_total_tc} TCs) have PASSED"
    for fidx1 in "${!compile_tc_array[@]}"
    do
        echo "    ${compile_tc_array[fidx1]}"
    done
}

###################################################
# SCRIPT USAGE GUIDE
###################################################
print_help(){
cat << EOF
QOS command script for ci

Synopsis
    ./run_qos_test.sh [OPTION]

Prerequisite
    1. please make sure that file below is properly configured according to your env.
        {POS_ROOT}/test/system/network/network_config.sh
    2. please make sure that pos binary exists on top of ${POS_ROOT}
    3. please configure your ip address, volume size, etc. propertly by editing nvme_fush_ci_test.sh

Description
    -v [target_volume to be created]
        default is 8
    -t [trtype]
        tcp:  IP configurations using tcp connection(default)
        rdma: IP configurations using rdma connection
    -a [target_system_ip]
        Default ip is 10.100.11.1
    -s [target_system_port]
        Default port is 1158
    -h
        Show script usage

Default configuration (if specific option not given)
    ./run_qos_test.sh -v 31 -t tcp -a 10.100.11.1 -s 1158

EOF
    exit 0
}

###################################################
# STARTS HERE
###################################################
# QoS Code Compilation & Sanity Checks
while getopts "v:t:a:s:p:m:l:h:" opt
do
    case "$opt" in
        v) NR_VOLUME="$OPTARG"
            ;;
        t) TRANSPORT="$OPTARG"
            ;;
        a) TARGET_IP="$OPTARG"
            ;;
        s) SUBSYSTEM="$OPTARG"
            ;;
        p) PORT="$OPTARG"
            ;;
        m) EXEC_MODE="$OPTARG"
	    ;;
        l) LOC="$OPTARG"
	    ;;
        h) print_help
            ;;
        ?) exit 2
            ;;
    esac
done
shift $(($OPTIND - 1))

if [ -z $LOC ]; then
LOC=HQ
fi

if [ ${LOC} == "SSIR" ];then
    DEFAULT_TRANSPORT=tcp
    DEFAULT_FABRIC_IP=133.133.133.25
    TARGET_SYSTEM_IP=107.109.113.29
    TARGET_USERNAME=root
    TARGET_PWD=siso@123
else
    DEFAULT_TRANSPORT=tcp
    DEFAULT_FABRIC_IP=10.100.11.5   # CI Server VM IP
    TARGET_SYSTEM_IP=10.1.11.5 #Set KHQ Target System IP
    TARGET_USERNAME=root
    TARGET_PWD=ibof
fi


log_normal "Checking variables..."
if [ -z $NR_VOLUME ]; then
NR_VOLUME=$DEFAULT_NR_VOLUME
fi

if [ -z $TRANSPORT ]; then
TRANSPORT=$DEFAULT_TRANSPORT
fi

if [ -z $TARGET_IP ]; then
TARGET_IP=$DEFAULT_FABRIC_IP
fi

if [ -z $SUBSYSTEM ]; then
SUBSYSTEM=$DEFAULT_SUBSYSTEM
fi

if [ -z $PORT ]; then
PORT=$DEFAULT_PORT
fi
if [ -z $EXEC_MODE ]; then
EXEC_MODE=1
fi


# Show the Test Setup Information
show_test_setup_info;

# Setup the Test Environment
setup_test_environment;

# Run all the QoS Test Cases
run_qos_test_cases;

exit 0

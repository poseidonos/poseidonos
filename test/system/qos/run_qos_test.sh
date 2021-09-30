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
detach_dev="unvme-ns-0"
spare_dev="unvme-ns-3"
VOLUME_SIZE=2147483648
NUM_REACTORS=31
DEFAULT_NR_VOLUME=31
DEFAULT_SUBSYSTEM=31
DEFAULT_PORT=1158
TARGET_ROOT_DIR=$(readlink -f $(dirname $0))/../../..
TARGET_SPDK_DIR=$TARGET_ROOT_DIR/lib/spdk
ibof_cli_old="$TARGET_ROOT_DIR/bin/poseidonos-cli"
ibof_cli="$TARGET_ROOT_DIR/bin/poseidonos-cli"
ARRAYNAME=POSArray
ARRAYNAME1=POSArray1
ARRAYNAME2=POSArray2
MINVALUEBWIOPS=10


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
    #texecc ${ibof_cli} array addspare --spare ${spare_dev} --array-name ${ARRAYNAME}
    texecc ${ibof_cli} array addspare --spare ${spare_dev} --array-name ${ARRAYNAME1}
    texecc ${ibof_cli} array addspare --spare ${spare_dev} --array-name ${ARRAYNAME2}
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
# WAIT FOR POS TO LAUNCH
#**************************************************************************
ibof_launch_wait(){
    retval=0
    n=1
    while [ $n -le 10 ]
    do
        texecc ${ibof_cli} system info --json-res | grep "\"description\":\"DONE\""
        texecc sleep 10s
        if [ $? -eq 0 ]; then
            retval=0
            break;
        else
            texecc sleep 5
            echo "Waiting for POS Launch"
            retval=1
        fi
        n=$(( n+1 ))
    done
    return $retval
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
stop_pos(){
    #texecc $TARGET_ROOT_DIR/bin/cli request unmount_ibofos
    
    texecc ${ibof_cli} array unmount --array-name ${ARRAYNAME} --force
    texecc sleep 10
    echo "Array successfully unmounted"
    #texecc ${ibof_cli} system stop --force
    texecc ${ibof_cli} system stop --force
    texecc sleep 10
    
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
# EXIT POS
#**************************************************************************
stop_pos_multi_array(){

    texecc ${ibof_cli} array unmount --array-name ${ARRAYNAME1} --force
    texecc ${ibof_cli} array unmount --array-name ${ARRAYNAME2} --force
    texecc sleep 10
    echo "Array successfully unmounted"
    echo ""
    texecc ${ibof_cli} system stop --force
    texecc sleep 10

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
start_ibofos(){
    texecc $TARGET_ROOT_DIR/test/regression/start_poseidonos.sh
    ibof_launch_wait
    EXPECT_PASS "POS OS Launch"  $?

    texecc sleep 10
    texecc $TARGET_ROOT_DIR/test/system/io_path/setup_ibofos_nvmf_volume.sh -c 1 -t $TRANSPORT -a $TARGET_IP -s $SUBSYSTEM -v $NR_VOLUME -u "unvme-ns-0,unvme-ns-1,unvme-ns-2" -p "unvme-ns-3"
    EXPECT_PASS "setup_ibofos_nvmf_volume.sh" $?

}

###################################################
# START POS WITH MULTI ARRAY and multi volume in a subsystem
###################################################
start_ibofos_with_multi_array(){
    texecc $TARGET_ROOT_DIR/test/regression/start_poseidonos.sh
    ibof_launch_wait
    EXPECT_PASS "POS OS Launch"  $?

    texecc sleep 10
    SUBSYSTEM_COUNT=2
    VOLUME_COUNT=8
    texecc $TARGET_ROOT_DIR/test/system/io_path/setup_multi_array.sh -c 1 -t $TRANSPORT -a $TARGET_IP -s $SUBSYSTEM_COUNT -v $VOLUME_COUNT
    EXPECT_PASS "setup_multi_array.sh" $?

}
###################################################
# PRINT FIO RESULTS
###################################################
print_fio_result()
{
    volId=$1
    readwrite=$2
    group=$3
    echo ""
    echo ""

    array=()
    while read line ; do
        array+=($line)
    done < <($INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py --volId="${volId}" --ioType="${readwrite}" --groupReport="${group}")
    echo ${array[@]}
}

###################################################
# LAUNCH FIO WITH INPUT CONFIGURATIONS
###################################################
launch_fio()
{
    if [ $# -ne 12 ];then
        echo "Insufficient  Parameters, ex. launch_fio file_num num_job io_depth bs readwrite runtime group workload run_background printValue volData"
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
    printValue=${10}
    volData=${11}
    multiarray=${12}
    echo -e "==========================================================="
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
        $INITIATOR_ROOT_DIR/test/system/qos/qos_fio_bench.py --file_num="${file_num}" --numjobs="${num_job}" --iodepth="${io_depth}" --bs="${bs}" --readwrite="${readwrite}" --run_time="${runtime}" --group_reporting="${group}" --workload_type="${workload}" --traddr="$TARGET_IP" --trtype="$TRANSPORT" --port="$PORT" --multiArray="${multiarray}" & >> $INITIATOR_ROOT_DIR/test/system/qos/qos_fio.log
    else
        $INITIATOR_ROOT_DIR/test/system/qos/qos_fio_bench.py --file_num="${file_num}" --numjobs="${num_job}" --iodepth="${io_depth}" --bs="${bs}" --readwrite="${readwrite}" --run_time="${runtime}" --group_reporting="${group}" --workload_type="${workload}" --traddr="$TARGET_IP" --trtype="$TRANSPORT" --port="$PORT" --multiArray="${multiarray}" >> $INITIATOR_ROOT_DIR/test/system/qos/qos_fio.log
    fi

    EXPECT_PASS "FIO Launch" $?
    if [ $printValue -eq 1 ]; then
        print_fio_result $volData $readwrite $group
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
    launch_fio 1 31 4 128k write 30 1 4 0 1 257 1
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
    launch_fio 1 31 4 128k read 30 1 4 0 1 257 1
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
    launch_fio 1 31 128 4k randwrite 30 1 4 0 1 257 1
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
    launch_fio 1 31 128 4k randread 30 1 4 0 1 257 1
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
    launch_fio 8 3 4 128k write 30 1 4 0 1 257 1
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
    launch_fio 8 3 4 128k read 30 1 4 0 1 257 1
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
    launch_fio 8 3 128 4k randwrite 30 1 4 0 1 257 1
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
    launch_fio 8 3 128 4k randread 30 1 4 0 1 257 1
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
    launch_fio 31 1 4 128k write 30 1 4 0 1 257 1
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
    launch_fio 31 1 4 128k read 30 1 4 0 1 257 1
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
    launch_fio 31 1 128 4k randwrite 30 1 4 0 1 257 1
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
    launch_fio 31 1 128 4k randread 30 1 4 0 1 257 1
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

#=========================================================
# Calculate Throttle Bandwidth in Mbps
#=========================================================
convertBwMbps()
{
    bwKBytes=$1
    mbpsFactor=1024
    bwMbps=`expr $bwKBytes / $mbpsFactor`
    return $bwMbps
}

#=========================================================
# Calculate Throttle Bandwidth in Mbps
#=========================================================
throttleBwMpbs()
{
    bwKBytes=$1
    convertBwMbps $bwKBytes
    bwMbps=$?
    divFactor=$2
    throttleMbps=`expr $bwMbps / $divFactor`
    if [ $throttleMbps == 0 ]; then
        throttleMbps=1
    fi
    return $throttleMbps
}


###################################################
# QOS READ WRITE WITH FE ENABLED
###################################################
tc_readwrite_fe()
{
    fio_tc_name="Type: Read Write FE, Volumes:8, Jobs:1, Details: QD(4), BS(128k), Sequential Write"
    volCnt=8
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    print_info "FIO Results without IOPS Throttling"
    launch_fio $volCnt 1 128 128k read 30 1 4 0 1 257 2
    array=()
    while read line ; do
        array+=($line)
    done < <($INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py --volId=257 --ioType="read" --groupReport=1)
    
    readBw=${array[1]}
    readIops=${array[3]}
    throttleFactor=`expr $volCnt \* 2`
    throttleBwMpbs $readBw $throttleFactor
    throttleBw=$?
    throttleIops=`expr ${readIops%.*} / $throttleFactor`

    #minimum iops suuported by Qos Cli is 10
    if [[ $throttleIops -le $MINVALUEBWIOPS ]]; then
        throttleIops=10
    fi

    if [[ $throttleBw -le $MINVALUEBWIOPS ]]; then
        throttleBw=10
    fi

    print_info "Throttling all the volumes, Bandwith at ${throttleBw}, IOPS at ${throttleIops}"
    volIdx=1
    while [ $volIdx -le $volCnt ]
    do
        #texecc $TARGET_ROOT_DIR/bin/cli qos vol_policy --vol vol$volIdx --maxbw $throttleBw --maxiops $throttleIops
        texecc $TARGET_ROOT_DIR/bin/poseidonos-cli qos create --volume-name vol$volIdx --maxbw $throttleBw --maxiops $throttleIops --array-name ${ARRAYNAME1}
        texecc $TARGET_ROOT_DIR/bin/poseidonos-cli qos create --volume-name vol$volIdx --maxbw $throttleBw --maxiops $throttleIops --array-name ${ARRAYNAME2}
        volIdx=`expr $volIdx + 1`
    done
    launch_fio $volCnt 1 4 128k write 30 1 4 0 1 257 2
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

###################################################
# QOS BANDWIDTH THROTTLE
###################################################
tc_bw_throttle()
{
    fio_tc_name="Type: BW Throttle, Volumes:8, Jobs:1, Details: QD(4), BS(128k), Sequential Write"
    volCnt=8
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    print_info "FIO Results without BW Throttling"
    launch_fio $volCnt 1 128 128k write 30 1 4 0 1 257 2
    array=()
    while read line ; do
        array+=($line)
    done < <($INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py --volId=257 --ioType="write" --groupReport=1)
    
    writeBw=${array[1]}
    throttleFactor=`expr $volCnt \* 2`
    throttleBwMpbs $writeBw $throttleFactor
    throttleBw=$?

    echo "ThrottleBw is"
    echo $throttleBw
    if [[ $throttleBw -le $MINVALUEBWIOPS ]]; then
        throttleBw=10
    fi

    print_info "Throttling all the volumes set at ${throttleBw}"
    volIdx=1
    while [ $volIdx -le $volCnt ]
    do
        texecc $TARGET_ROOT_DIR/bin/poseidonos-cli qos create --volume-name vol$volIdx --maxbw $throttleBw --array-name ${ARRAYNAME1}
        texecc $TARGET_ROOT_DIR/bin/poseidonos-cli qos create --volume-name vol$volIdx --maxbw $throttleBw --array-name ${ARRAYNAME2}
        volIdx=`expr $volIdx + 1`
    done
    launch_fio $volCnt 1 4 128k write 30 1 4 0 1 257 2
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}

###################################################
# QOS IOPS THROTTLE
###################################################
tc_iops_throttle()
{
    fio_tc_name="Type: IOPS Throttle, Volumes:8, Jobs:1, Details: QD(4), BS(128k), Sequential Write"
    volCnt=8
    show_tc_info "${fio_tc_name}"
    start_tc "${fio_tc_name}"
    print_info "FIO Results without IOPS Throttling"
    launch_fio $volCnt 1 4 128k write 30 1 4 0 1 257 2
    array=()
    while read line ; do
        array+=($line)
    done < <($INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py --volId=257 --ioType="write" --groupReport=1)
    
    writeIops=${array[3]}
    throttleFactor=`expr $volCnt \* 2`
    throttleIops=`expr ${writeIops%.*} / $throttleFactor`
    
    #minimum iops supported by qos is 10
    if [[ $throttleIops -le $MINVALUEBWIOPS ]]; then
        throttleIops=10
    fi
    print_info "Throttling all the volumes set at ${throttleIops}"
    volIdx=1
    while [ $volIdx -le $volCnt ]
    do
        texecc $TARGET_ROOT_DIR/bin/poseidonos-cli qos create --volume-name vol$volIdx --maxiops $throttleIops --array-name ${ARRAYNAME1}
        texecc $TARGET_ROOT_DIR/bin/poseidonos-cli qos create --volume-name vol$volIdx --maxiops $throttleIops --array-name ${ARRAYNAME2}
        volIdx=`expr $volIdx + 1`
    done

    launch_fio $volCnt 1 4 128k write 30 1 4 0 1 257 2
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
}
###################################################
# QOS 1 VOLUME MINIMUM IOPS
###################################################
tc_throttle_value_check()
{
    tc_name="Throttle Value check"
    show_tc_info "${tc_name}"
    start_tc "${tc_name}"
    volIdx=1
    volCnt=8
    throttleIops=10
    while [ $volIdx -le $volCnt ]
    do
        texecc $TARGET_ROOT_DIR/bin/poseidonos-cli qos create --volume-name vol$volIdx --maxiops $throttleIops --array-name ${ARRAYNAME1}
        texecc $TARGET_ROOT_DIR/bin/poseidonos-cli qos create --volume-name vol$volIdx --maxiops $throttleIops --array-name ${ARRAYNAME2}
        volIdx=`expr $volIdx + 1`
    done
    #as the unit is in KIOPS
    throttleIopsToCheck=10000
    launch_fio $volCnt 1 4 128k write 30 0 4 0 0 257 2

    volIdx=1
    while [ $volIdx -le $volCnt ]
    do
        array=()
        while read line ; do
            array+=($line)
        done < <($INITIATOR_ROOT_DIR/test/system/qos/fio_output_parser.py --volId=$volIdx  --ioType="write" --groupReport=0)
        writeIopsAfterThrottle=${array[3]}
        writeIopsInInt=${writeIopsAfterThrottle%.*}
        echo ""
        #echo "WriteIopsAfterthrottle"
        #echo $writeIopsInInt
        if [[ $writeIopsInInt -gt $throttleIopsToCheck ]]; then
            echo "IOPS test failed"
        fi
        volIdx=`expr $volIdx + 1`
    done
    EXPECT_PASS "${tc_name}" $?
    end_tc "${tc_name}"
}

###################################################
# QOS 1 VOLUME MINIMUM IOPS
###################################################
tc_list_qos_policies()
{
    tc_name="List QoS Policies"
    show_tc_info "${tc_name}"
    start_tc "${tc_name}"
    volIdx=1
    volCnt=8
    while [ $volIdx -le $volCnt ]
    do
        texecc $TARGET_ROOT_DIR/bin/poseidonos-cli qos list --volume-name vol$volIdx --array-name ${ARRAYNAME1}
        texecc $TARGET_ROOT_DIR/bin/poseidonos-cli qos list --volume-name vol$volIdx --array-name ${ARRAYNAME2}
        volIdx=`expr $volIdx + 1`
    done
    end_tc "${fio_tc_name}"
}
###################################################
# QOS UNMOUNT VOLUME TEST
###################################################
unmount_volume_test()
{
    tc_name="Unmount volume test"
    show_tc_info "${tc_name}"
    start_tc "${tc_name}"
    volIdx=1
    volCnt=8
    ##while [ $volIdx -le $volCnt ]
    ##do
        ##texecc $TARGET_ROOT_DIR/bin/poseidonos-cli volume unmount --volume-name vol$volIdx --array-name ${ARRAYNAME1}
        ##volIdx=`expr $volIdx + 1`
    ##done

    texecc $TARGET_ROOT_DIR/bin/poseidonos-cli array unmount  --array-name ${ARRAYNAME1} --json-res --force
    texecc sleep 10
    
    #mount volumes and arrays back
    texecc $TARGET_ROOT_DIR/bin/poseidonos-cli array mount --array-name ${ARRAYNAME1} --json-res
    texecc sleep 10
    volIndex=1
    while [ $volIndex -le $volCnt ]
    do
        texecc $TARGET_ROOT_DIR/bin/poseidonos-cli volume mount  --volume-name vol$volIndex --array-name ${ARRAYNAME1}
        texecc $TARGET_ROOT_DIR/bin/poseidonos-cli qos create --volume-name vol$volIndex  --array-name ${ARRAYNAME1} --maxiops 10

        volIndex=`expr $volIndex + 1`
    done

    launch_fio $volCnt 1 4 128k write 30 1 4 0 1 257 2
    EXPECT_PASS "${fio_tc_name}" $?
    end_tc "${fio_tc_name}"
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
                    tc_31v_1job_write tc_31v_1job_read tc_31v_1job_randwrite tc_31v_1job_randread)
    elif [ $mode == "fe_qos_multi_array" ]; then
        print_info "FE QOS TEST CASES MULTI ARRAY"
        echo ""
        tc_array=(tc_readwrite_fe tc_bw_throttle tc_iops_throttle tc_throttle_value_check tc_list_qos_policies unmount_volume_test)
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
# ENABLE FE QOS
###################################################
enable_fe_qos(){
    texecc $TARGET_ROOT_DIR/test/system/qos/fe_qos_config.py -s true -f true -i low
    texecc sleep 10s
}

###################################################
# DISABLE FE QOS
###################################################
disable_fe_qos(){
    texecc $TARGET_ROOT_DIR/test/system/qos/fe_qos_config.py -s true -f false -i low
    texecc sleep 10s
}

###################################################
# CODE COMPILATION
###################################################
#compile_pos(){
    #commented this as compile is never called with an argument
    #compile_option=$1
    #if [ $compile_option -eq  "pos" ]; then
        #texecc make clean
	    #texecc make -j$(nproc)
        #echo "compilation starts"
    #else
    #texecc $TARGET_ROOT_DIR/script/build_ibofos.sh
    #fi
#}

###################################################
# TEST CASES
###################################################
with_be_qos(){
    tc_name="POS Code, BE Enabled, FE Disabled"
    show_tc_info "${tc_name}"
    start_tc "${tc_name}"
    disable_fe_qos
    start_ibofos
    EXPECT_PASS "Successful POS Launch & Configuration" $?
    run_fio_tests be_qos with_be_qos
    EXPECT_PASS "Successful Completion of FIO test cases" $?
    texecc sleep 10
    EXPECT_PASS "${tc_name}" $?
    end_tc "${tc_name}"
    stop_pos
}

with_fe_qos(){
    tc_name="POS Code, BE Enabled, FE Enabled"
    echo ""
    echo ""
    echo $tc_name
    show_tc_info "${tc_name}"
    start_tc "${tc_name}"
    enable_fe_qos
    start_ibofos
    EXPECT_PASS "Successful POS Launch & Configuration" $?
    run_fio_tests fe_qos with_fe_qos
    EXPECT_PASS "Successful Completion of FIO test cases" $?
    texecc sleep 10
    EXPECT_PASS "${tc_name}" $?
    disable_fe_qos
    end_tc "${tc_name}"
    stop_pos
}



with_fe_qos_multi_array(){
    tc_name="POS Code, BE Enabled, FE Enabled. MULTI ARRAY"
    echo ""
    echo ""
    echo $tc_name
    show_tc_info "${tc_name}"
    start_tc "${tc_name}"
    enable_fe_qos
    start_ibofos_with_multi_array
    EXPECT_PASS "Successful POS Launch & Configuration" $?
    run_fio_tests fe_qos_multi_array with_fe_qos
    EXPECT_PASS "Successful Completion of FIO test cases" $?
    texecc sleep 10
    EXPECT_PASS "${tc_name}" $?
    disable_fe_qos
    end_tc "${tc_name}"
    stop_pos_multi_array
}
###################################################
# SANITY TESTS
###################################################
run_qos_test_cases(){
    echo "----------------------------------------------------------------"
    echo "Test Cases To Run POS Code with BE/ FE QoS"
    echo "----------------------------------------------------------------"
    tc_array_one=(with_be_qos)
    total_tc=${compile_tc_array[@]}
    local tc_list=""

    for fidx in "${!tc_array_one[@]}"
    do
         echo ${tc_array_one[fidx]}
         ${tc_array_one[fidx]}
         tc_list+="   $((fidx+1)): ${tc_array_one[fidx]}\n"
    done
    print_notice "All QOS (${total_tc} TCs) have PASSED"
    for fidx1 in "${!tc_array_one[@]}"
    do
        echo "    ${tc_array_one[fidx1]}"
    done
}
###################################################
# SANITY TESTS WITH MULTI ARRAY
###################################################
run_multi_array_test(){
    echo "----------------------------------------------------------------"
    echo "Test Cases To Run POS Code with BE/ FE QoS"
    echo "----------------------------------------------------------------"
    tc_array_one=(with_fe_qos_multi_array)
    total_tc=${compile_tc_array[@]}
    local tc_list=""

    for fidx in "${!tc_array_one[@]}"
    do
         echo ${tc_array_one[fidx]}
         ${tc_array_one[fidx]}
         tc_list+="   $((fidx+1)): ${tc_array_one[fidx]}\n"
    done
    print_notice "All QOS (${total_tc} TCs) have PASSED"
    for fidx1 in "${!tc_array_one[@]}"
    do
        echo "    ${tc_array_one[fidx1]}"
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
        {IBOFOS_ROOT}/test/system/network/network_config.sh
    2. please make sure that ibofos binary exists on top of ${IBOFOS_ROOT}
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
LOC=SSIR
fi

if [ ${LOC} == "SSIR" ];then
    DEFAULT_TRANSPORT=tcp
    TARGET_FABRIC_IP=111.100.13.175
    TARGET_SYSTEM_IP=107.109.113.29
    TARGET_USERNAME=root
    TARGET_PWD=siso@123
else
    DEFAULT_TRANSPORT=tcp
    TARGET_FABRIC_IP=10.100.11.5   # CI Server VM IP
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
TARGET_IP=$TARGET_FABRIC_IP
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

#run_test_on_multi_array and multi_volume_on_1 subsystem
run_multi_array_test;


exit 0

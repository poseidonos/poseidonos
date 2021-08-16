#!/bin/bash
ROOT_DIR=$(readlink -f $(dirname $0))/../../
SPDK_DIR=$ROOT_DIR/lib/spdk/
SETUP_NVMf_PATH=test/system/io_path/setup_ibofos_nvmf_volume.sh
network_config_file=test/system/network/network_config.sh
CONFIG_FILE=/etc/pos/pos.conf
ORIG_CONFIG_FILE=/etc/pos/pos_orig.conf
DEVICES=()
globalLogFile="/var/log/pos/nvme_flush_test.log"
localLogDirPath=$ROOT_DIR/test/system/nvme_flush/
logfile=$globalLogFile

cd $ROOT_DIR

#************************ DEFAULT VALUES ************************
DEFAULT_TARGET_USERNAME=root
DEFAULT_TARGET_PASSWORD=ibof
DEFAULT_TARGET_ROOT_DIR=$ROOT_DIR

DEFAULT_TRANSPORT=tcp
DEFAULT_TARGET_IP="127.0.0.1"
DEFAULT_FABRIC_IP=10.100.11.5   # CI Server VM IP

DEFAULT_MAX_VOLUMES=3
DEFAULT_MAX_SUBSYSTEMS=$DEFAULT_MAX_VOLUMES
DEFAULT_PORT_NUM=1158
DEFAULT_VOLUME_SIZE=21474836480
#****************************************************************

TARGET_USERNAME=$DEFAULT_TARGET_USERNAME
TARGET_PASSWORD=$DEFAULT_TARGET_PASSWORD
TARGET_ROOT_DIR=$DEFAULT_TARGET_ROOT_DIR
TARGET_SPDK_DIR=$TARGET_ROOT_DIR/lib/spdk/

TRANSPORT=$DEFAULT_TRANSPORT
TARGET_IP=$DEFAULT_TARGET_IP
TARGET_FABRIC_IP=$DEFAULT_FABRIC_IP

MAX_VOLUMES=$DEFAULT_MAX_VOLUMES
MAX_SUBSYSTEMS=$DEFAULT_MAX_SUBSYSTEMS
PORT_NUM=$DEFAULT_PORT_NUM
VOLUME_SIZE=$DEFAULT_VOLUME_SIZE

target_dev_list="unvme-ns-0,unvme-ns-1,unvme-ns-2"
target_spare_dev="unvme-ns-3"
write_data_size="10G"
partial_stripe_size="508K" #Less than one stripe data size"

#To add more use cases add extra element to each array below
total_usecases=2
use_case_description=("Single-subsystem,Single-volume" "Multi-subsystem,Multi-volume")
num_volumes_list=(1 $MAX_VOLUMES)
#If total usecases exceed 5 add more pattern
patterns=(0xfeedbabe 0xdeadcafe 0xbeedfeed 0xfeebdabe 0xeeeeaaaa)

totalFlushFails=0
dataMismatch=0

exec_mode=0 #Default run in loopback

#**************************************************************************
#Execution in target
#**************************************************************************
texecc()
{
    if [[ $exec_mode -eq 0 ]]; then
        sudo $@
    else
        sshpass -p $TARGET_PASSWORD ssh -q -tt -o StrictHostKeyChecking=no -q $TARGET_USERNAME@${TARGET_IP} "cd ${TARGET_ROOT_DIR}; sudo $@"
    fi
}

#**************************************************************************
#Clear exisitng log file
#**************************************************************************
cleanup()
{
    texecc rm -f /etc/pos/core/*.core
    texecc rm -f ${logfile}
    texecc touch ${logfile}

    mkdir -p $localLogDirPath
    for usecase in $(seq 0 $((total_usecases-1)))
    do
        rm -f $localLogDirPath/usecase$((usecase+1)).log
    done
}

#**************************************************************************
#Check for all rdma modules
#**************************************************************************
network_module_check()
{
    texecc test/regression/network_module_check.sh >> ${logfile}
}

#**************************************************************************
#Install prerequisite modules
#**************************************************************************
check_env()
{
    if [ ! -f /usr/sbin/nvme ]; then
        sudo apt install -y nvme-cli &> /dev/null
        if [ ! -f /usr/sbin/nvme ]; then
            echo "Prerequisities installation failed"
            exit 2;
        fi
    fi

    if [ ! -f /usr/bin/sshpass ]; then
        sudo apt install -y sshpass &> /dev/null
        if [ ! -f /usr/bin/sshpass ]; then
            echo "Prerequisities installation failed"
            exit 2;
        fi
    fi
}

#**************************************************************************
#Check for connections
#**************************************************************************
setup_prerequisite()
{
    chmod +x $ROOT_DIR/script/*.sh
    chmod +x $ROOT_DIR/$network_config_file
    chmod +x $ROOT_DIR/$SETUP_NVMf_PATH

    texecc chmod +x script/*.sh >> ${logfile}
    texecc chmod +x ${network_config_file} >> ${logfile}
    texecc chmod +x $SETUP_NVMf_PATH

    texecc ls /sys/class/infiniband/*/device/net >> ${logfile}
    ls /sys/class/infiniband/*/device/net >> ${logfile}

    if [[ ${TRANSPORT} = "rdma" || ${TRANSPORT} = "RDMA" ]]; then
        echo "  RDMA configuration for server..."
        texecc ./${network_config_file} server >> ${logfile}
        wait
        echo "  Done" >> ${logfile}

        echo "  RDMA configuration for client..."
        $ROOT_DIR/${network_config_file} client >> ${logfile}
        wait
        echo "  Done" >> ${logfile}
    fi
}

#**************************************************************************
#Setup arrays and controllers
#**************************************************************************
setup_POS()
{
    NUM_VOLUMES=$1
    NUM_SUBSYSTEMS=$2
    clean=$3

    texecc $SETUP_NVMf_PATH -c $clean -t $TRANSPORT -a $TARGET_FABRIC_IP -s $NUM_SUBSYSTEMS -v $NUM_VOLUMES -S $VOLUME_SIZE &>> ${logfile}
}

#**************************************************************************
#Conect the POS volumes as NVMf target devices
#**************************************************************************
connect_devices()
{
    num_subsystems=$1

    for i in $(seq 1 $num_subsystems)
    do
        sudo nvme connect -t $TRANSPORT -a $TARGET_FABRIC_IP -s $PORT_NUM -n nqn.2019-04.pos:subsystem$i >> ${logfile}
        if [[ $? -ne 0 ]]; then
            echo "Failed to connect devices using nvmf. Check ip."
            exit_CItest_on_Failure $num_subsystems
        fi
    done

    target_devices=($(sudo nvme list | grep -E 'SPDK|POS|pos' | awk '{print $1}'))

    for device in "${target_devices[@]}"
    do
        DEVICES+=($device)
    done

    echo "  POS devices connected through NVMf"

    sleep 2
}

#**************************************************************************
#Disconect the POS volumes as NVMf target devices
#**************************************************************************
disconnect_nvmf_controllers()
{
    num_subsystems=$1
    echo "  Disconnecting nvmf controllers"
    for i in $(seq 1 $num_subsystems)
    do
        sudo nvme disconnect -n nqn.2019-04.pos:subsystem$i >> ${logfile}
    done
}

#**************************************************************************
#Start POS and setup the arrays and volumes (clean/retain)
#**************************************************************************
start_POS()
{
    num_volumes=$1
    num_subsystems=$2
    clean=$3

    echo "Starting POS" >> ${logfile}
    texecc test/regression/start_poseidonos.sh >> ${logfile}
    #POS working condition checked using nvme connect
    #after setup of arrays and volumes
    sleep 10

    setup_POS $num_volumes $num_subsystems $clean
    sleep 2s
}

#**************************************************************************
#Exit POS for NPOR
#**************************************************************************
exit_POS()
{
    echo "  Unmounting and Exiting POS"
    texecc ./bin/poseidonos-cli array unmount --array-name POSArray --force >> ${logfile}
    texecc ./bin/poseidonos-cli system stop --force >> ${logfile}

    texecc ps -C poseidonos > /dev/null >> ${logfile}
    while [[ ${?} == 0 ]]
    do
        texecc sleep 1s
        texecc ps -C poseidonos > /dev/null >> ${logfile}
    done
    echo "  POS exited successfully"
}

#**************************************************************************
#Kill POS
#**************************************************************************
kill_POS()
{
    echo "  Kill POS"
    texecc ./test/script/kill_poseidonos.sh >> ${logfile}

    texecc ps -C poseidonos > /dev/null >> ${logfile}
    while [[ ${?} == 0 ]]
    do
        texecc sleep 1s
        texecc ps -C poseidonos > /dev/null >> ${logfile}
    done
}

#**************************************************************************
#Do SPOR
#**************************************************************************
do_spor()
{
    if [[ $exec_mode -eq 0 ]]; then
        res_flush_gcov=`./bin/poseidonos-cli --json-res wbt flush_gcov | tr -d '\n' | sed -e 's/\"/\"/g'`
    else
        res_flush_gcov=`sshpass -p $TARGET_PASSWORD ssh -q -tt -o StrictHostKeyChecking=no -q $TARGET_USERNAME@${TARGET_IP} "cd ${TARGET_ROOT_DIR}; sudo ./bin/poseidonos-cli --json-res wbt flush_gcov | tr -d '\n' | sed -e 's/\"/\"/g'"`
    fi
    python test/system/nvme_flush/flush_gcov.py "$res_flush_gcov" >> ${logfile}

    kill_POS

    texecc ./script/backup_latest_hugepages_for_uram.sh >>${logfile}
}

exit_CItest_on_Failure()
{
    num_subsystems=$1

    disconnect_nvmf_controllers $num_subsystems

    kill_POS

    echo "NVMe flush command failed"
    exit 1
}

#**************************************************************************
#Issue NVMe Flush Command
#**************************************************************************
flush_data()
{
    isBackground=$1
    if [ ${isBackground} -eq 1 ]; then
        echo "Flushing data of all volumes in background(in parallel to write operations)" >> ${logfile}
    else
        echo "Flushing data of all volumes" >> ${logfile}
    fi

    successFlushes=0

    vol_num=0
    for device in "${DEVICES[@]}"
    do
        if [ ${isBackground} -eq 1 ]; then
            failFlushes[$vol_num]=`$ROOT_DIR/test/system/nvme_flush/nvme_flush.sh $device & >> ${logfile}`
        else
            nvme flush $device -n 1 2> /dev/null | grep -q 'success' && successFlushes=1

            totalFlushFails=$((totalFlushFails+1-successFlushes))
        fi
        vol_num=$((vol_num+1))
    done

    if [ ${isBackground} -eq 1 ];then
        echo "  Waiting for Write and Flush operations to complete"
        wait
        for i in $(seq 0 $((vol_num-1)))
        do
            totalFlushFails=$((totalFlushFails+failFlushes[${i}]))
        done
    fi
}

#**************************************************************************
#Issue write operation(s)
#**************************************************************************
write_data()
{
    num_subsystems=$1
    total_data_to_write=$2
    isBackground=$3

    for i in $(seq 1 $num_subsystems)
    do
        pattern_num=$((i-1))
        sudo $ROOT_DIR/test/regression/fio_pattern.py -t ${TRANSPORT} -i "${TARGET_FABRIC_IP}" -p ${PORT_NUM} -n $i -s $total_data_to_write -v ${patterns[$((i-1))]} -w write &>> ${logfile} &
    done

    if [ ${isBackground} -eq 0 ]; then
        wait
    fi

    sleep 2
}

#****************************************************************************************
#Issue read operation(s) on previously written data in BG and verify the data is matching
#****************************************************************************************
read_data_and_verify()
{
    num_subsystems=$1
    total_data_to_read=$2
    
    for i in $(seq 1 $num_subsystems)
    do
        pattern_num=$((i-1))
        $ROOT_DIR/test/regression/fio_pattern.py -t ${TRANSPORT} -i "${TARGET_FABRIC_IP}" -p ${PORT_NUM} -n $i -v ${patterns[$pattern_num]} -w read -s $total_data_to_read -d 1 &>> ${logfile} &
    done

    wait

    for i in $(seq 1 $num_volumes)
    do
        grep "verify failed" $ROOT_DIR/verify$i.json | head -n 1 >> ${logfile}
        res=$?
        if [ ${res} -eq 0 ]; then
           if [[ ! -f $ROOT_DIR/verify$i.json ]]; then
                echo "No data" >> ${logfile}
                dataMismatch=1
                break
            fi
            echo "Data matching" >> ${logfile}
        else
            echo "Data mismatch" >> ${logfile}
            dataMismatch=1
            break
        fi
    done

    for i in $(seq 1 $num_volumes)
    do
        rm -f $ROOT_DIR/verify$i.json >> ${logfile}
    done
}

#****************************************************************************************
#Copy the usecase logs to global log file
#****************************************************************************************
copy_usecase_logs()
{
    for usecase in $(seq 0 $((total_usecases-1)))
    do
        cat $localLogDirPath/usecase$((usecase+1)).log >> $globalLogFile
    done
}

#****************************************************************************************
#Set flush in config file
#****************************************************************************************
enable_flush()
{
    texecc cp $CONFIG_FILE $ORIG_CONFIG_FILE
    echo " Enabling flush command handling in POS"
    texecc -s eval "jq -r '.flush.enable |= true' $CONFIG_FILE > /tmp/temp.json && mv /tmp/temp.json $CONFIG_FILE"
}

#****************************************************************************************
#Reset flush in config file
#****************************************************************************************
disable_flush()
{
    echo "  Restoring the original config file"
    texecc mv $ORIG_CONFIG_FILE $CONFIG_FILE
}

#**************************************************************************
#Main Function:
#Multiple usecases: ("Single-subsystem,Single-volume" "Multi-subsystem,Multi-volume")
#Multiple testcases: ("Flush after POS init" "Flush after write operation" "Flush and Write operations in parallel" "Verify data after flush operation")
#**************************************************************************
start_test_cases()
{
    totalFlushFails=0
    cleanup

    #Call the network_module_check.sh script
    network_module_check

    check_env
    setup_prerequisite

    disconnect_nvmf_controllers $MAX_SUBSYSTEMS

    #Kill any instances of POS runnning
    kill_POS

    print_help

    enable_flush

    for usecase in $(seq 0 $((total_usecases-1)))
    do
        echo "Usecase " $((usecase+1)) " Description: " ${use_case_description[$usecase]}
        num_volumes=${num_volumes_list[$usecase]}
        num_subsystems=$num_volumes

        #Seperate log files for each usecase
        logfile=$localLogDirPath/usecase$((usecase+1)).log

        #*********************************************************************************
        #Setting up systems
        #*********************************************************************************
        echo "  Start POS with clean setup"
        #Do a clean setup
        clean_setup=1
        start_POS $num_volumes $num_subsystems $clean_setup

        echo "  POS started successfully and array setup and volume mount successful"

        #Connect devices through NVMf
        connect_devices $num_subsystems
        sleep 2
        echo "Listing devices enumerated from POS" >> ${logfile}
        nvme list >> ${logfile}

        #*********************************************************************************
        #Test case 1: Flush after POS init
        #Description: To test if NVMe flush command works
        #             after initialization of POS and NVMf connections
        #             (No dirty entries created yet)
        #*********************************************************************************
        echo "  Test Case 1: Flush on init of POS success"
        isBackground=0
        flush_data $isBackground #$num_volumes

        if [[ $totalFlushFails -ne 0 ]];then
            exit_CItest_on_Failure $num_subsystems
        fi

        echo "  Test Case 1 Completed: Flush on init of POS success"
        sleep 5

        #*********************************************************************************
        #Test case 2: Flush and write operations in parallel
        #Description: To test if NVMe flush command works
        #             when a write operation is in progress
        #*********************************************************************************
        echo "  Test Case 2: Parallel Writes and Flush"
        isBackground=1
        total_data_to_write=$write_data_size
        write_data $num_subsystems $total_data_to_write $isBackground
        flush_data $isBackground $num_volumes

        if [[ $totalFlushFails -ne 0 ]];then
            exit_CItest_on_Failure $num_subsystems
        fi

        echo "  Test Case 2 Completed: Parallel Writes and Flush" 
        sleep 5

        #*********************************************************************************
        #Test case 3: Flush after write operation
        #Description: To test if NVMe flush command works after write operation is complete
        #*********************************************************************************
        echo "  Test Case 3: Flush after write"
        total_data_to_write=$partial_stripe_size
        isBackground=0
        write_data $num_subsystems $total_data_to_write $isBackground
        flush_data $isBackground $num_volumes
        echo "  Test Case 3 Completed: Flush after write"
        sleep 5

        #*********************************************************************************
        #Test case 4: Verify data after flush operation
        #Description: To test the data and meta flushed by NVMe flush command /
        #             is in persistent medium by using SPO /
        #             and bringup with previous configuration
        #*********************************************************************************
        echo "  Test case 4: Data verification using SPOR"
        #Bring up POS with previous configurations (SPOR)
        do_spor

        echo "  Bringup POS with previous configuration"
        sleep 3
        clean_setup=0
        start_POS $num_volumes $num_subsystems $clean_setup

        echo "Verify data after SPOR" >> ${logfile}
        total_data_to_read=$partial_stripe_size
        read_data_and_verify $num_subsystems $total_data_to_read

        if [[ $dataMismatch -eq 1 ]];then
            exit_CItest_on_Failure $num_subsystems
        fi

        echo "  Test case 4 Completed: Data verification complete after SPOR"
        
        disconnect_nvmf_controllers $num_subsystems

        exit_POS
        DEVICES=()
    done

    disable_flush

    copy_usecase_logs
}

print_help()
{
cat << EOF
NVMe Flush command script for ci

Synopsis
    ./nvme_flush_ci_test.sh [OPTION]

Prerequisite
    1. please make sure that file below is properly configured according to your env.
        {IBOFOS_ROOT}/test/system/network/network_config.sh
    2. please make sure that poseidonos binary exists on top of {IBOFOS_ROOT}
    3. please configure your ip address, volume size, etc. propertly by editing nvme_flush_ci_test.sh

Description
    -x [exec_mode]
        0: loopback(default)
        1: through NVMf connection
    -t [trtype]
        tcp:  IP configurations using tcp connection(default)
        rdma: IP configurations using rdma connection
    -i [target_ip]
        Default ip is 127.0.0.1 
    -f [target_fabric_ip]
        Default ip is 10.100.1.25
    -s [target_system_port]
        Default port is 1158
    -v [max_num_volumes]
        Default max_numvolumes = 3
    -u [target_username]
        Default target username = "root"
    -p [target_password]
        Default target password = "ibof"
    -l [target_POS_root_dir]
        Default location is ${TARGET_ROOT_DIR}
    -h
        Show script usage

Present running configuration
    ./nvme_flush_ci_test.sh -x ${exec_mode} -t ${TRANSPORT} -i ${TARGET_IP} -f ${TARGET_FABRIC_IP} -s ${PORT_NUM} -v ${MAX_VOLUMES} -u ${TARGET_USERNAME} -p ${TARGET_PASSWORD} -l ${TARGET_ROOT_DIR}

EOF
}

while getopts "x:t:i:f:s:hv:u:p:l:" opt
do
    case "$opt" in
        x) exec_mode=$OPTARG
            if [[ $exec_mode -lt 0 || $exec_mode -gt 1 ]]; then
                print_help
                exit 2
            fi
            ;;
        t) TRANSPORT="$OPTARG"
            ;;
        i) TARGET_IP="$OPTARG"
            ;;
        f) TARGET_FABRIC_IP="$OPTARG"
            ;;
        s) PORT_NUM="$OPTARG"
            ;;
        v) MAX_VOLUMES="$OPTARG"
            ;;
        u) TARGET_USERNAME="$OPTARG"
            ;;
        p) TARGET_PASSWORD="$OPTARG"
            ;;
        l) TARGET_ROOT_DIR="$OPTARG"
            ;;
        h) print_help
           exit 0
            ;;
        ?) exit 2
            ;;
    esac
done

start_test_cases
    
echo "CI for Flush completed successfully"

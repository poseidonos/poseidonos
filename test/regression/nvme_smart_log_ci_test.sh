#!/bin/bash
ROOT_DIR=$(readlink -f $(dirname $0))/../../
SPDK_DIR=$ROOT_DIR/lib/spdk/
SETUP_NVMf_PATH=test/system/io_path/setup_ibofos_nvmf_volume.sh
network_config_file=test/system/network/network_config.sh
CONFIG_FILE=/etc/pos/pos.conf
ORIG_CONFIG_FILE=/etc/pos/pos_orig.conf
DEVICES=()
globalLogFile="/var/log/ibofos/nvme_smart_log_command.log"
localLogDirPath=$ROOT_DIR/test/system/nvme_smart_log/
logfile=$globalLogFile

cd $ROOT_DIR

#************************ DEFAULT VALUES ************************
DEFAULT_TARGET_USERNAME=root
DEFAULT_TARGET_PASSWORD=ibof
DEFAULT_TARGET_ROOT_DIR=$ROOT_DIR

DEFAULT_TRANSPORT=tcp
DEFAULT_TARGET_IP="127.0.0.1"
DEFAULT_FABRIC_IP=111.100.13.175  # CI Server VM IP

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
read_data_size="10G"

#To add more use cases add extra element to each array below
total_usecases=2
use_case_description=("Single-subsystem,Single-volume" "Multi-subsystem,Multi-volume")
num_volumes_list=(1 $MAX_VOLUMES)
#If total usecases exceed 5 add more pattern
patterns=(0xfeedbabe 0xdeadcafe 0xbeedfeed 0xfeebdabe 0xeeeeaaaa)

total_smart_fails=0
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
        sshpass -p $TARGET_PASSWORD ssh -tt -o StrictHostKeyChecking=no -q $TARGET_USERNAME@${TARGET_IP} "cd ${TARGET_ROOT_DIR}; sudo $@"
    fi
}

#**************************************************************************
#Clear exisitng log file
#**************************************************************************
cleanup()
{
    texecc rm -f /etc/ibofos/core/*.core
    texecc rm -f ${logfile}
    texecc touch ${logfile}

    if [[ -d $localLogDirPath ]]; then
        for usecase in $(seq 0 $((total_usecases-1)))
        do
            rm -f $localLogDirPath/usecase$((usecase+1)).log
        done
    else
        mkdir $localLogDirPath
    fi
    #cleanup the output file created by smart log command
    rm $ROOT_DIR/test/system/nvme_smart_log/OUTPUT
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
    
     target_nvme=`sudo nvme list | grep -E 'SPDK|pos|POS' | awk '{print $1}' | head -n 1`
    echo $target_devices
    for device in "${target_devices[@]}"
    do
        echo $device
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
 
    echo "  Setting up POS now..."
    setup_POS $num_volumes $num_subsystems $clean
    #adding device name print to check for nvme connection
    target_devices=($(sudo nvme list | grep -E 'SPDK|POS|pos' | awk '{print $1}'))

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
#exit CI test on failure
#**************************************************************************

exit_CItest_on_Failure()
{
    num_subsystems=$1

    disconnect_nvmf_controllers $num_subsystems

    kill_POS

    echo "NVMe smart log command failed"
    exit 1
}

#**************************************************************************
#Issue NVMe smart log Command
#**************************************************************************
issue_smart_log_command()
{
    isBackground=$1
    field_type=$2
    # 1 : temperature 2 :read_commands 3:write commands 4:bytes read 5: bytes written 
    if [ ${isBackground} -eq 1 ]; then
        echo "Issuing smart log command on all volumes in background(in parallel to IO operations)" >> ${logfile}
    else
        echo "issuing smart log command in foreground on all volumes" >> ${logfile}
    fi

    vol_num=0
    for device in "${DEVICES[@]}"
    do
        if [ ${isBackground} -eq 1 ]; then
            output_file_path=test/system/nvme_smart_log 
            file_path=$ROOT_DIR/$output_file_path
            failed_smart_log_cmds[$vol_num]=`$ROOT_DIR/test/system/nvme_smart_log/nvme_smart_log.sh $device $field_type $file_path & >> ${logfile}`
        
        else
            file_path=$ROOT_DIR/test/system/nvme_smart_log/OUTPUT
            rm -rf $file_path
            nvme smart-log $device -n 1 -o json > $file_path
            res=$(python $ROOT_DIR/test/system/nvme_smart_log/parse_smart_output.py $field_type $file_path)
            result=$?
            
            if [ ${field_type} -eq 1 ]; then
                if [ ${res} -eq 0 ]; then #the value was filled as 0
                    failed_smart_log_count=$(failed_smart_log_count+1)
                fi
            fi
            
            if [ ${result} -eq 1 ]; then #the json parsing failed
                failed_smart_log_count=$((failed_smart_log_count+1))
            fi
            
            
            if [ ${field_type} -eq 2 ]; then
                read_commands=$res
            elif [ ${field_type} -eq 3 ]; then
                write_commands=$res
            elif [ ${field_type} -eq 4 ]; then
                bytes_read=$res
            elif [ ${field_type} -eq 5 ]; then
                bytes_written=$res
            fi

        fi
        vol_num=$((vol_num+1))
    done

    if [ ${isBackground} -eq 1 ];then
        echo "  Waiting for Write and smart log operations to complete"
        wait
        for i in $(seq 0 $((vol_num-1)))
        do
            total_smart_fails=$((failed_smart_log_count+failed_smart_log_cmds[${i}]))
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
#**************************************************************************
#Issue read operation(s)
#**************************************************************************
read_data()
{
    num_subsystems=$1
    total_data_to_write=$2
    isBackground=$3

    for i in $(seq 1 $num_subsystems)
    do
        pattern_num=$((i-1))
        sudo $ROOT_DIR/test/regression/fio_pattern.py -t ${TRANSPORT} -i "${TARGET_FABRIC_IP}" -p ${PORT_NUM} -n $i -s $total_data_to_read -v ${patterns[$((i-1))]} -w read &>> ${logfile} &
    done

    if [ ${isBackground} -eq 0 ]; then
        wait
    fi

    sleep 2
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
enable_smartlog()
{
    texecc cp $CONFIG_FILE $ORIG_CONFIG_FILE
    echo " Enabling smart command handling in POS"
    texecc -s eval "jq -r '.admin.smart_log_page |= true' $CONFIG_FILE > /tmp/temp.json && mv /tmp/temp.json $CONFIG_FILE"
}

#****************************************************************************************
#Reset flush in config file
#****************************************************************************************
disable_smartlog()
{
    echo "  Restoring the original config file"
    texecc mv $ORIG_CONFIG_FILE $CONFIG_FILE
}

#**************************************************************************
#Main Function:
#Multiple usecases: ("Single-subsystem,Single-volume" "Multi-subsystem,Multi-volume")
#Multiple testcases: ("Smart log after POS init" "Smart Log and Write operations in parallel" "no of write bytes","no of read bytes","Persisten storahe of metadata")
#**************************************************************************
start_test_cases()
{
    total_smart_fails=0
    cleanup

    #Call the network_module_check.sh script
    network_module_check

    check_env
    setup_prerequisite

    disconnect_nvmf_controllers $MAX_SUBSYSTEMS

    #Kill any instances of POS runnning
    kill_POS

    print_help
    
    enable_smartlog

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
        #Test case 1:Smart log afte POS init
        #Description: To test if NVMe smart log command works
        #             after initialization of POS and NVMf connections
        #             and data is returned(as spdk doesnt support smart log, if data is returned in any of the fileds then it is coming from POS
        #*********************************************************************************
        echo ""
        echo "  Test Case 1: Smart Log on init of POS"
        isBackground=0
        field_type=1
        echo "  Issuing smart Log command"
        issue_smart_log_command $isBackground $field_type

        if [[ $total_smart_fails -ne 0 ]];then
            exit_CItest_on_Failure $num_subsystems
        fi

        echo "  Test Case 1 Completed: Smart Log on init of POS success"
        echo ""
        sleep 5

        #*********************************************************************************
        #Test case 2: smart log and write operations in parallel
        #Description: To test if NVMe smart log command works
        #             when a write operation is in progress
        #*********************************************************************************
        echo ""
        echo "  Test Case 2: Parallel Writes and smart Logs"
        isBackground=1
        field_type=1
        total_data_to_write=$write_data_size
        write_data $num_subsystems $total_data_to_write $isBackground
        issue_smart_log_command $isBackground $fieldtype $num_volumes

        if [[ $total_smart_fails -ne 0 ]];then
            exit_CItest_on_Failure $num_subsystems
        fi

        echo "  Test Case 2 Completed: Parallel Writes and Smart Log" 
        echo ""
        sleep 5

        #*********************************************************************************
        #Test case 3: Smart log after write operation
        #Description: To test if NVMe smart log command works after write operation is complete and data bytes written/no of write commands are returned accordingly
        #*********************************************************************************
        echo ""
        echo "  Test Case 3: No of write bytes"
        total_data_to_write=$write_data_size
        GB_to_Bytes=1024*1024*1024
        total_data_to_write_in_bytes=10*GB_to_Bytes
        block_size=4096
        nvme_spec_unit=512000
        data_units_written=2*total_data_to_write_in_bytes/nvme_spec_unit
        data_units_written=$((data_units_written+1))
        #write_commands_count=$((2*total_data_to_write_in_bytes/block_size))
        isBackground=0
        fieldtype=5
        bytes_written=0
        write_data $num_subsystems $total_data_to_write $isBackground
        issue_smart_log_command $isBackground $fieldtype $num_volumes
        echo "  Writebytes from smart log $bytes_written"
        echo "  write_bytes calculated $data_units_written"
        if [[ $bytes_written -le 0 ]];then
            exit_CItest_on_Failure $num_subsystems
        fi
        echo "  Test Case 3 Completed: No of write bytes "
        echo ""
        sleep 5

        #*********************************************************************************
        #Test case 4: Smart log after read operation
        #Description: To test if NVMe smart log command works after read operation is complete and data bytes read/no of read commands are returned accordingly
        #*********************************************************************************
        echo ""
        echo "  Test case 4: No of read bytes"
        #Bring up POS with previous configurations (SPOR)
        total_data_to_read=$read_data_size
        GB_to_Bytes=1024*1024*1024
        total_data_to_read_in_bytes=10*GB_to_Bytes
        block_size=4096
        #read_commands_count=total_data_to_read_in_bytes/block_size
        nvme_spec_unit=512000
        data_units_read=total_data_to_read_in_bytes/nvme_spec_unit
        data_units_read=$(($data_units_read+3))
        #extra 46 commands to account for the read commands sent during nvme connect
        #read_commands_count=$(($read_commands_count+46))
        isBackground=0
        fieldtype=4
        #bytes_read=0
        bytes_read=0
        read_data $num_subsystems $total_data_to_write $isBackground
        issue_smart_log_command $isBackground $fieldtype $num_volumes
        echo "  Read bytes from smart Log $bytes_read"
        echo "  Read bytes calculated $data_units_read"
        if [[ $bytes_read -le 0 ]];then
            exit_CItest_on_Failure $num_subsystems
        fi
        echo "  Test Case 4 Completed: No of read bytes "
        echo ""
        sleep 5

        #*********************************************************************************
        #Test case 5: Smart log after meta storage
        #Description: To test if NVMe smart log command works after NPOR of POS. And returns the previously stored no of bytes and no of commands for both read and write.
        #*********************************************************************************
        echo ""
        echo "  Test case 5: Persistent storage of data"
        #store bytes read before npor
        bytes_read=0
        fieldtype=4
        isBackground=0
        issue_smart_log_command $isBackground $fieldtype $num_volumes
        bytes_read_before_npor=$bytes_read
        #extra 2 units to account for reads sent for connect issud after POS comes up
        bytes_read_before_npor=$(($bytes_read_before_npor+2))
        echo "  Read bytes before NPOR $bytes_read_before_npor"
        #store bytes_written before npor
        bytes_written=0
        fieldtype=5
        isBackground=0
        issue_smart_log_command $isBackground $fieldtype $num_volumes
        bytes_written_before_npor=$bytes_written
        echo "  Write bytes before NPOR $bytes_written_before_npor"
        
        #do npor
        echo "  Clean exit from POS"
        disconnect_nvmf_controllers $num_subsystems
        exit_POS
        #bringup POS with previous configuration
        echo "  Bringup POS with previous configuration"
        sleep 3
        clean_setup=0
        start_POS $num_volumes $num_subsystems $clean_setup
        connect_devices $num_subsystems

        echo "  Enlisting devices now from Nvme List "
        for device in "${target_devices[@]}"
        do
            echo " $device"
        done
        
        echo "  POS up succesfully with previous configuration"
        #store data read after npor
        bytes_read=0
        fieldtype=4
        isBackground=0
        issue_smart_log_command $isBackground $fieldtype $num_volumes
        bytes_read_after_npor=$bytes_read
        echo "  Read bytes after NPOR $bytes_read_after_npor"
        #store bytes_written after npor
        bytes_written=0
        fieldtype=5
        isBackground=0
        issue_smart_log_command $isBackground $fieldtype $num_volumes
        bytes_written_after_npor=$bytes_written
        echo "  write bytes after NPOR $bytes_written_after_npor"
        if [[ $bytes_written_after_npor -le 0 ]];then
            echo "failing from write"
            exit_CItest_on_Failure $num_subsystems
        fi
        if [[ $bytes_read_after_npor -le 0 ]];then
            echo "failing from read"
            exit_CItest_on_Failure $num_subsystems
        fi

        echo "  Test case 5 completed: Persistent storage of data "
        echo ""
        sleep 5

        disconnect_nvmf_controllers $num_subsystems

        exit_POS
        DEVICES=()
    done

    disable_smartlog

    copy_usecase_logs
}

print_help()
{
cat << EOF
NVMe Smart Log command script for ci

Synopsis
    ./nvme_smart_log_ci_test.sh [OPTION]

Prerequisite
    1. please make sure that file below is properly configured according to your env.
        {IBOFOS_ROOT}/test/system/network/network_config.sh
    2. please make sure that ibofos binary exists on top of {IBOFOS_ROOT}
    3. please configure your ip address, volume size, etc. propertly by editing nvme_smart_log_ci_test.sh

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
    ./nvme_smart_log_ci_test.sh -x ${exec_mode} -t ${TRANSPORT} -i ${TARGET_IP} -f ${TARGET_FABRIC_IP} -s ${PORT_NUM} -v ${MAX_VOLUMES} -u ${TARGET_USERNAME} -p ${TARGET_PASSWORD} -l ${TARGET_ROOT_DIR}

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
    
echo "CI for Smart Log completed successfully"

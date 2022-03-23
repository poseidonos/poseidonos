tcCount=0
tcTotalCount=0

support_max_subsystem=1

exec_mode=
target_nvme=
manual_ibofos_run_mode=0
nvme_cli="nvme"
nss="nqn.2019-04.pos:subsystem"
trtype=""
port="1158"
target_dev_list="unvme-ns-0,unvme-ns-1,unvme-ns-2"

MBtoB=$((1024*1024))
GBtoB=$((1024*${MBtoB}))

ibof_phy_volume_size_mb=102400
ibof_phy_volume_size_byte=$((${ibof_phy_volume_size_mb}*${MBtoB}))

test_volume_size_byte=$((2*1024*1024*1024)) # 2GB
max_io_range_byte=$((1024*1024*1024)) # 1GB
max_io_boundary_byte=$((${test_volume_size_byte} - ${max_io_range_byte}))
blk_offset=0
io_blk_cnt=0
blk_size_kb=64 # 64 KB
max_io_boundary_blk=$((${max_io_boundary_byte}/1024/${blk_size_kb}-1))
max_io_range_blk=$((${max_io_range_byte}/1024/${blk_size_kb}))

io_blk_cnt=`shuf -i 1-${max_io_range_blk} -n 1`
max_io_boundary_blk=$((${max_io_boundary_blk}-${io_blk_cnt}))
blk_offset=`shuf -i 0-${max_io_boundary_blk} -n 1`

connectList=(0)

#######################################################################
# Section A
#######################################################################

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

    if [ $expectedResult -eq 0 ]; then
        echo -e "\033[1;34m${date} [result] ${result} \033[0m" 1>&2;
    else
        echo -e "\033[1;41m${date} [TC failed] ${result} \033[0m" 1>&2;
    fi
}

pause()
{
    echo "Press any key to continue.."
    read -rsn1
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

texecc()
{
    case ${exec_mode} in
    0) # loop-back test
        cd ${rootTarget}; $@
        ;;
    1) # over-fabric test
        sshpass -p bamboo ssh root@${target_ip} "cd ${rootTarget}; sudo $@"
        ;;
    2) # script verification test
        echo "sshpass -p bamboo ssh root@${target_ip} \"cd ${rootTarget}; sudo $@\""
        ;;
    esac
}

iexecc()
{
    case ${exec_mode} in
    0) # loop-back test
        cd ${rootInit}; $@
        ;;
    1) # over-fabric test
        cd ${rootInit}; sudo $@
        ;;
    2) # script verification test
        echo sudo "$@"
        ;;
    esac
    
}

show_tc_info()
{
    local tcName=$1
    print_notice "Information for \"${tcName}\""
    echo -e "  - Number of supported subsystems: ${support_max_subsystem}"
    echo -e "  - Type of transportation for remote devices: ${trtype}"
    echo -e "  - Target IP: ${target_ip}"
    echo -e "  - Target fabric IP: ${target_fabric_ip}"
    echo -e "  - Root dir (init): ${rootInit}"
    echo -e "  - Root dir (target): ${rootTarget}"
}

disconnect_nvmf_contollers()
{
    local volNum=$1

    connectList[${volNum}]=0

    print_info "Disconnecting ${nss}${volNum}"
    iexecc ${nvme_cli} disconnect -n ${nss}${volNum}
}

kill_pos()
{
    texecc ./test/script/kill_poseidonos.sh &>> ${logfile}
    sleep 2

    for i in `seq 1 ${support_max_subsystem}`
    do
        disconnect_nvmf_contollers ${i}
    done
}

create_subsystem()
{
    for i in `seq 1 ${support_max_subsystem}`
    do
        connect_port=${port}
        print_info "Creating subsystem ${nss}$i, ip ${target_fabric_ip}, port ${connect_port}"
        texecc ${spdk_rpc_script} nvmf_create_subsystem ${nss}$i -a -s POS0000000000000$i -d POS_VOL_$i #>> ${logfile}

        print_info "Adding listener ${nss}$i, ip ${target_fabric_ip}, port ${connect_port}"
        texecc ${spdk_rpc_script} nvmf_subsystem_add_listener ${nss}$i -t TCP -a ${target_fabric_ip} -s ${connect_port} #>> ${logfile}

        connectList+=(0)
    done
}

create_array()
{
    print_info "create array"

    iexecc rm -rf create_array.txt result.txt

    create_array=0
    # if [ ! -z $1 ] && [ $1 == "create" ]; then
    if [ $1 == "create" ]; then
        create_array=1
    fi

    texecc ./bin/poseidonos-cli device scan

    if [ $create_array -eq 1 ]; then
        print_info "Target device list=${target_dev_list}"
        texecc ./bin/poseidonos-cli devel resetmbr
        texecc ./bin/poseidonos-cli array create -a POSArray -b uram0 -d ${target_dev_list} --json-res > create_array.txt
    else
        texecc echo "{\"Response\":{\"result\":{\"status\":{\"code\":0}}}}" > create_array.txt
    fi

    iexecc cat create_array.txt | jq ".Response.result.status.code" > result.txt
    result=$(<result.txt)

    if [ "$result" == "" ]; then
        print_result "there is a problem" 1
        iexecc cat create_array.txt
        iexecc rm -rf create_array.txt result.txt
        return 1
    elif [ $result -ne 0 ]; then
        print_result "failed to create/load array" 1
        iexecc cat create_array.txt
        iexecc rm -rf create_array.txt result.txt
        return 1
    fi

    iexecc rm -rf create_array.txt result.txt

    return 0
}

mount_array()
{
    print_info "mount array"

    iexecc rm -rf mount_array.txt result.txt

    texecc ./bin/poseidonos-cli array mount -a POSArray --json-res > mount_array.txt

    iexecc cat mount_array.txt | jq ".Response.result.status.code" > result.txt
    result=$(<result.txt)

    if [ "$result" == "" ]; then
        print_result "there is a problem" 1
        iexecc cat mount_array.txt
        iexecc rm -rf mount_array.txt result.txt
        return 1
    elif [ $result -ne 0 ]; then
        print_result "failed to mount array" 1
        iexecc cat mount_array.txt
        iexecc rm -rf mount_array.txt result.txt
        return 1
    fi

    iexecc rm -rf mount_array.txt result.txt

    return 0
}

unmount_array()
{
    print_info "unmount array"

    iexecc rm -rf unmount_array.txt result.txt

    texecc ./bin/poseidonos-cli array unmount -a POSArray --json-res --force > unmount_array.txt

    iexecc cat unmount_array.txt | jq ".Response.result.status.code" > result.txt
    result=$(<result.txt)

    if [ "$result" == "" ]; then
        print_result "there is a problem" 1
        iexecc cat unmount_array.txt
        iexecc rm -rf unmount_array.txt result.txt
        return 1
    elif [ $result -ne 0 ]; then
        print_result "failed to unmount array" 1
        iexecc cat unmount_array.txt
        iexecc rm -rf unmount_array.txt result.txt
        return 1
    fi

    iexecc rm -rf unmount_array.txt result.txt

    return 0
}

delete_array()
{
    print_info "delete array"

    iexecc rm -rf delete_array.txt result.txt

    texecc ./bin/poseidonos-cli array delete -a POSArray --json-res --force > delete_array.txt

    iexecc cat delete_array.txt | jq ".Response.result.status.code" > result.txt

    result=$(<result.txt)

    if [ "$result" == "" ]; then
        print_result "there is a problem" 1
        iexecc cat delete_array.txt
        iexecc rm -rf delete_array.txt result.txt
        return 1
    elif [ $result -ne 0 ]; then
        print_result "failed to delete array" 1
        iexecc cat delete_array.txt
        iexecc rm -rf delete_array.txt result.txt
        return 1
    fi

    iexecc rm -rf delete_array.txt result.txt

    return 0
}

create_volume()
{
    print_info "create volume [vol name: $1, byte size: $2B, max iops: $3, max bw: $4]"

    iexecc rm -rf create_volume.txt result.txt

    #issue
    texecc ./bin/poseidonos-cli volume create -v $1 --size $2 --maxiops $3 --maxbw $4 -a POSArray --json-res > create_volume.txt

    # check response
    iexecc cat create_volume.txt | jq '.Response.result.status.code' > result.txt
    result=$(<result.txt)

    #check
    if [ ${result} -ne 0 ]; then
        print_result "failed to create a volume" 1
        iexecc cat create_volume.txt
        iexecc rm -rf create_volume.txt result.txt
        return 1
    fi

    iexecc rm -rf create_volume.txt result.txt

    return 0
}

normal_shutdown()
{
    print_info "Shutting down normally"

    iexecc rm -rf shutdown.txt result.txt

    unmount_array;
    result=$?

    if [ $result != 0 ]; then
        print_result "failed to unmount array." 1
        return 1
    fi

    texecc ./bin/poseidonos-cli system stop --force --json-res --force > shutdown.txt
    
    ps -C poseidonos > /dev/null
    while [[ ${?} == 0 ]]
    do
        sleep 1s
        ps -C poseidonos > /dev/null
    done

    iexecc cat shutdown.txt | jq ".Response.result.status.code" > result.txt
    result=$(<result.txt)

    if [ "$result" == "" ]; then
        print_result "there is a problem" 1
        iexecc cat shutdown.txt
        iexecc rm -rf shutdown.txt result.txt
        return 1
    elif [ ${result} != 0 ]; then
        print_result "Failed to shutdown" 1
        iexecc rm -rf shutdown.txt result.txt
        return 1
    fi

    for i in `seq 1 ${support_max_subsystem}`
    do
        disconnect_nvmf_contollers ${i}
    done

    print_info "Shutdown has been completed!"

    iexecc rm -rf shutdown.txt result.txt

    return 0
}

graceful_shutdown()
{
    print_info "Shutting down gracefully in few seconds..."

    iexecc rm -rf shutdown.txt

    for i in `seq 1 ${support_max_subsystem}`
    do
        disconnect_nvmf_contollers ${i}
    done

    if [ -z $1 ]; then
        unmount_array;
        result=$?

        if [ $result != 0 ]; then
            print_result "failed to unmount array." 1
            return 1
        fi
    fi

    texecc ./bin/poseidonos-cli system stop --force --json-res --force > shutdown.txt

    ps -C poseidonos > /dev/null
    while [[ ${?} == 0 ]]
    do
        sleep 1s
        ps -C poseidonos > /dev/null
    done

    iexecc rm -rf shutdown.txt

    print_info "Shutdown has been completed!"
}

abrupt_shutdown()
{
    local withBackup=$1

    print_info "Shutting down suddenly in few seconds..."

    kill_pos

    if [ "${withBackup}" != "" ]; then
        texecc ./script/backup_latest_hugepages_for_uram.sh
        sleep 3
    fi

    for i in `seq 1 ${support_max_subsystem}`
    do
        disconnect_nvmf_contollers ${i}
    done

    print_info "Shutdown has been completed!"
}

discover_n_connect_nvme_from_initiator()
{
    local volNum=$1
    local nssName="${nss}$1"

    if [ ${connectList[${volNum}]} == 1 ]; then
        return 0
    fi

    connectList[${volNum}]=1

    connect_port=${port}
    iexecc ${nvme_cli} discover -t ${trtype} -a ${target_fabric_ip} -s ${connect_port}

    print_info "Connecting ${nssName}"

    target_nvme=`sudo nvme list | grep -E 'SPDK|POS|pos' | awk '{print $1}' | head -16 | sed -n ''${volNum}',1p'`
    if [[ "${target_nvme}" == "" ]] || ! ls ${target_nvme} > /dev/null ; then
        connect_port=${port}
        iexecc ${nvme_cli} connect -t ${trtype} -n ${nssName} -a ${target_fabric_ip} -s ${connect_port}
        res=$?
        if [ ${res} -ne 0 ]; then
            print_result "NVMe device cannot be connected" 1
            return 1
        fi
    fi

    return 0
}

create_base()
{
    print_info "Creating nvmf transport"
    texecc ${spdk_rpc_script} nvmf_create_transport -t TCP -b 64 -n 4096 #>> ${logfile}

    print_info "Creating bdev for nvram"
    texecc ${spdk_rpc_script} bdev_malloc_create -b uram0 1024 512 #>> ${logfile}

    return 0
}

start_telemetry()
{
    print_info "Starting telmetry..."
    texecc ./bin/poseidonos-cli telemetry start
    sleep 5
    print_info "Now telemetry is available"
}

start_pos()
{
    texecc rm -rf /dev/shm/ibof_nvmf_trace.pid*

    print_info "Starting pos..."
    texecc ./test/regression/start_poseidonos.sh

    sleep 5 # takes longer if pos accesses actual drives
    print_info "Now pos is running..."

    start_telemetry

    return 0
}

update_config()
{
    # path
    logfile="${rootInit}/script/ft_test.log"
    spdk_rpc_script="${rootInit}/lib/spdk/scripts/rpc.py"
}

#######################################################################
# Section B
#######################################################################

bringup_pos()
{
    start_pos;

    create_base;

    create_array $1
    result=$?

    if [ $result != 0 ]; then
        print_result "failed to create/load array." 1
        return 1
    fi

    mount_array;
    result=$?

    if [ $result != 0 ]; then
        print_result "failed to mount array." 1
        return 1
    fi

    connectList=(0)
    create_subsystem

    print_info "Bring-up PoseidonOS done!"

    return 0
}

npor_and_check_volumes()
{
    iexecc rm -rf npor_and_check_volumes0.txt npor_and_check_volumes1.txt result0.txt result1.txt

    print_info "npor and check volumes"

    #get status
    texecc ./bin/poseidonos-cli volume list -a POSArray --json-res > npor_and_check_volumes0.txt
    iexecc cat npor_and_check_volumes0.txt | jq '.Response.result.data.volumes | length' > result0.txt
    result0=$(<result0.txt)

    normal_shutdown;
    result=$?

    if [ $result -ne 0 ]; then
        return 1
    fi

    bringup_pos 0;
    result=$?

    if [ $result != 0 ]; then
        iexecc cat npor_and_check_volumes0.txt
        iexecc rm -rf npor_and_check_volumes0.txt npor_and_check_volumes1.txt result0.txt result1.txt
        return 1
    fi

    #get status
    texecc ./bin/poseidonos-cli volume list -a POSArray --json-res > npor_and_check_volumes1.txt
    iexecc cat npor_and_check_volumes1.txt | jq '.Response.result.data.volumes | length' > result1.txt
    result1=$(<result1.txt)

    #check
    if [ ${result0} -eq ${result1} ]; then
        print_result "passed." 0
        iexecc rm -rf npor_and_check_volumes0.txt npor_and_check_volumes1.txt result0.txt result1.txt
        return 0
    else
        print_result "failed - # of volumes" 1
        echo -e "before **"
        iexecc cat npor_and_check_volumes0.txt
        echo -e "after **"
        iexecc cat npor_and_check_volumes1.txt
        iexecc rm -rf npor_and_check_volumes0.txt npor_and_check_volumes1.txt result0.txt result1.txt
    fi

    return 1
}

spor_and_check_volumes()
{
    iexecc rm -rf spor_and_check_volumes0.txt spor_and_check_volumes1.txt result0.txt result1.txt

    print_info "npor and check volumes"

    #get status
    texecc ./bin/poseidonos-cli volume list -a POSArray --json-res > spor_and_check_volumes0.txt
    iexecc cat spor_and_check_volumes0.txt | jq '.Response.result.data.volumes | length' > result0.txt
    result0=$(<result0.txt)

    abrupt_shutdown 1;
    bringup_pos 0;
    result=$?

    if [ $result != 0 ]; then
        iexecc cat spor_and_check_volumes0.txt
        iexecc rm -rf spor_and_check_volumes0.txt spor_and_check_volumes1.txt result0.txt result1.txt
        return 1
    fi

    #get status
    texecc ./bin/poseidonos-cli volume list -a POSArray --json-res > spor_and_check_volumes1.txt
    iexecc cat spor_and_check_volumes1.txt | jq '.Response.result.data.volumes | length' > result1.txt
    result1=$(<result1.txt)

    #check
    if [ ${result0} -eq ${result1} ]; then
        print_result "passed." 0
        iexecc rm -rf npor_and_check_volumes0.txt spor_and_check_volumes1.txt result0.txt result1.txt
        return 0
    else
        print_result "failed - # of volumes" 1
        echo -e "before **"
        iexecc cat npor_and_check_volumes0.txt
        echo -e "after **"
        iexecc cat spor_and_check_volumes1.txt
        iexecc rm -rf spor_and_check_volumes0.txt spor_and_check_volumes1.txt result0.txt result1.txt
    fi

    return 1
}

mount_and_check()
{
    #param
    local volNum=$1
    local volName="vol$1"

    iexecc rm -rf mount_and_check.txt result.txt

    print_info "mount and check [vol name: ${volName}]"

    #issue
    texecc ./bin/poseidonos-cli volume mount -v ${volName} -a POSArray

    #get status
    texecc ./bin/poseidonos-cli volume list -a POSArray --json-res > mount_and_check.txt
    iexecc cat mount_and_check.txt | jq -c --arg vol ${volName} '.Response.result.data.volumes[] | select(.name == $vol) | .status' > result.txt
    result=$(<result.txt)

    #check
    if [ "$result" == "" ]; then
        print_result "volume is not existed" 1
        echo -e "result **"
        iexecc cat mount_and_check.txt
        iexecc rm -rf mount_and_check.txt result.txt
        return 1
    elif [ "$result" == "\"Mounted\"" ]; then
        print_result "volume mounted" 0
    else
        print_result "failed" 1
        echo -e "result **"
        iexecc cat mount_and_check.txt
        iexecc rm -rf mount_and_check.txt result.txt
        return 1
    fi

    if [ -z $2 ]; then
        discover_n_connect_nvme_from_initiator ${volNum}
    fi

    res=$?
    if [ ${res} -ne 0 ]; then
        return 1
    fi

    iexecc rm -rf mount_and_check.txt result.txt

    return 0
}

unmount_and_check()
{
    #param
    local volNum=$1
    local volName="vol$1"

    iexecc rm -rf unmount_and_check.txt result.txt

    print_info "unmount and check [vol name: ${volName}]"

    disconnect_nvmf_contollers ${volNum}

    #issue
    texecc ./bin/poseidonos-cli volume unmount -v ${volName} -a POSArray --force

    #get status
    texecc ./bin/poseidonos-cli volume list -a POSArray --json-res > unmount_and_check.txt
    iexecc cat unmount_and_check.txt | jq -c --arg vol ${volName} '.Response.result.data.volumes[] | select(.name == $vol) | .status' > result.txt
    result=$(<result.txt)

    #check
    if [ "$result" == "" ]; then
        print_result "volume is not existed" 1
        echo -e "result **"
        iexecc cat unmount_and_check.txt
        rm -rf unmount_and_check.txt result.txt
        return 1
    elif [ "$result" == "\"Unmounted\"" ]; then
        rm -rf unmount_and_check.txt result.txt
        print_result "volume unmounted" 0
    else
        print_result "failed" 1
        echo -e "result **"
        iexecc cat unmount_and_check.txt
        rm -rf unmount_and_check.txt result.txt
        return 1
    fi

    rm -rf unmount_and_check.txt result.txt

    return 0
}

create_and_check()
{
    #param
    local volName="vol$1"
    local byteSize=$2
    local maxIops=$3
    local maxBw=$4

    iexecc rm -rf create_and_check0.txt create_and_check1.txt result0.txt result1.txt

    print_info "create and check [vol name: ${volName}, byte size: ${byteSize}, max iops: ${maxIops}, max bw: ${maxBw}]"

    # get pre-condition
    texecc ./bin/poseidonos-cli volume list -a POSArray --json-res > create_and_check0.txt
    iexecc cat create_and_check0.txt | jq '.Response.result.data.volumes | length' > result0.txt
    result0=$(<result0.txt)

    #issue
    create_volume $volName $byteSize $maxIops $maxBw
    result=$?

    if [ $result != 0 ]; then
        iexecc rm -rf create_and_check0.txt create_and_check1.txt result0.txt result1.txt
        return 1
    fi

    #get status
    texecc ./bin/poseidonos-cli volume list -a POSArray --json-res > create_and_check1.txt
    iexecc cat create_and_check1.txt | jq '.Response.result.data.volumes | length' > result1.txt
    result1=$(<result1.txt)

    #check
    if [ ${result0} -lt ${result1} ]; then
        print_result "passed." 0
    else
        print_result "failed - creation" 1
        echo -e "before **"
        iexecc cat create_and_check0.txt
        echo -e "after **"
        iexecc cat create_and_check1.txt
        iexecc rm -rf create_and_check0.txt create_and_check1.txt result0.txt result1.txt
        return 1
    fi

    iexecc rm -rf create_and_check0.txt create_and_check1.txt result0.txt result1.txt

    return 0
}

delete_and_check()
{
    #param
    local volName="vol$1"

    iexecc rm -rf delete_and_check0.txt delete_and_check1.txt result0.txt result1.txt

    print_info "delete and check [vol name: ${volName}]"

    # get pre-condition
    texecc ./bin/poseidonos-cli volume list -a POSArray --json-res > delete_and_check0.txt
    iexecc cat delete_and_check0.txt | jq '.Response.result.data.volumes | length' > result0.txt
    result0=$(<result0.txt)

    #issue
    texecc ./bin/poseidonos-cli volume delete -v ${volName} -a POSArray --json-res --force > delete_and_check1.txt

    # check response
    iexecc cat delete_and_check1.txt | jq '.Response.result.status.code' > result1.txt
    result1=$(<result1.txt)

    #check
    if [ ${result1} -ne 0 ]; then
        print_result "failed - deletion" 1
        iexecc cat delete_and_check1.txt
        return 1
    fi

    #get status
    texecc ./bin/poseidonos-cli volume list -a POSArray --json-res > delete_and_check1.txt
    iexecc cat delete_and_check1.txt | jq '.Response.result.data.volumes | length' > result1.txt
    result1=$(<result1.txt)

    #check
    if [ ${result0} -gt ${result1} ]; then
        print_result "passed." 0
    else
        print_result "failed - deletion" 1
        echo -e "before **"
        iexecc cat delete_and_check0.txt
        echo -e "after **"
        iexecc cat delete_and_check1.txt
        iexecc rm -rf delete_and_check0.txt delete_and_check1.txt result0.txt result1.txt
        return 1
    fi

    iexecc rm -rf delete_and_check0.txt delete_and_check1.txt result0.txt result1.txt

    return 0
}

change_volume_name_and_check()
{
    local volName=$1
    local newVolName=$2

    iexecc rm -rf change_volume_name_and_check.txt result.txt

    print_info "change volume name [vol name: ${volName} to ${newVolName}]"

    texecc ./bin/poseidonos-cli volume rename -v ${volName} -n ${newVolName} -a POSArray --json-res > change_volume_name_and_check.txt
    iexecc cat change_volume_name_and_check.txt | jq '.Response.result.status.code' > result.txt
    result=$(<result.txt)

        #check
    if [ ${result} != 0 ]; then
        print_result "failed to change the volume name." 1
        iexecc cat change_volume_name_and_check.txt
        iexecc rm -rf change_volume_name_and_check.txt result.txt

        return 1
    fi

    iexecc rm -rf change_volume_name_and_check.txt result.txt

    return 0
}

parallel_dd()
{
    local input_file=$1
    local output_file=$2
    local blk_size_kb=$3
    local blk_count=$4
    local skip_blk_offset=$5 # input offset
    local seek_blk_offset=$6 # output offset

    local total_io_cnt_kb=$((${blk_size_kb}*${blk_count}))
    local parallel_io_size_kb=$((128*1024)) # 128 (MB) * 1024

    if [ ${total_io_cnt_kb} -gt ${parallel_io_size_kb} ]; then
        local io_count_at_once=$((${parallel_io_size_kb}/${blk_size_kb}))
        local remaining_io_cnt_kb=${total_io_cnt_kb}
        local curr_seek_blk_offset=${seek_blk_offset/${blk_size_kb}}
        local curr_skip_blk_offset=${skip_blk_offset/${blk_size_kb}}

        while [ ${remaining_io_cnt_kb} -gt ${parallel_io_size_kb} ]; do
            iexecc dd if=${input_file} of=${output_file} bs=${blk_size_kb}K count=${io_count_at_once} seek=${curr_seek_blk_offset} skip=${curr_skip_blk_offset} conv=sparse,notrunc oflag=nocache status=none &
            remaining_io_cnt_kb=$((${remaining_io_cnt_kb}-${parallel_io_size_kb}))
            curr_seek_blk_offset=$((${curr_seek_blk_offset}+${io_count_at_once}))
            curr_skip_blk_offset=$((${curr_skip_blk_offset}+${io_count_at_once}))
        done
        # handle last remaining portion
        local last_remaining_blk_cnt=$((${remaining_io_cnt_kb}/${blk_size_kb}))
        iexecc dd if=${input_file} of=${output_file} bs=${blk_size_kb}K count=${last_remaining_blk_cnt} seek=${curr_seek_blk_offset} skip=${curr_skip_blk_offset} conv=sparse,notrunc oflag=nocache status=none &
        wait
    else
        local curr_seek_blk_offset=${seek_blk_offset/${blk_size_kb}}
        local curr_skip_blk_offset=${skip_blk_offset/${blk_size_kb}}
        iexecc dd if=${input_file} of=${output_file} bs=${blk_size_kb}K count=${blk_count} seek=${curr_seek_blk_offset} skip=${curr_skip_blk_offset} conv=sparse,notrunc oflag=nocache status=none &
        wait
    fi
}

write_data()
{
    local volNum=$1
    local volName="vol$1"

    blk_offset=${blk_offset}
    io_blk_cnt=${io_blk_cnt}
    blk_unit_size_kb=${blk_size_kb}

    file_postfix=".dat"
    write_file="write_"${volName}${file_postfix}
    read_file="read_"${volName}${file_postfix}
    cmp_file="cmp_"${volName}${file_postfix}

    iexecc touch ${write_file}
    iexecc dd if=/dev/urandom of=${write_file} bs=1024K count=1024 seek=0 skip=0 conv=sparse,notrunc oflag=nocache status=none &
    wait

    target_nvme=`sudo nvme list | grep -E 'SPDK|POS|pos' | awk '{print $1}' | head -16 | sed -n ''${volNum}',1p'`
    if [[ "${target_nvme}" == "" ]] || ! ls ${target_nvme} > /dev/null ; then
        print_result "NVMe drive is not found..." 1
        return 1
    fi

    print_info "Write data: target=${target_nvme}, blk offset=${blk_offset}, blk cnt=${io_blk_cnt}, blk size=${blk_unit_size_kb}KB"

    local remained_io_kb=$((${io_blk_cnt}*${blk_unit_size_kb}))
    local parallel_io_size_kb=$((128*1024)) # 128 (MB) * 1024
    local io_count_at_once=$((${parallel_io_size_kb}/${blk_unit_size_kb}))
    local current_seek_blk=${blk_offset}
    local current_skip_blk=0
    local amount_once_kb=${blk_unit_size_kb}

    while [ ${remained_io_kb} -gt ${parallel_io_size_kb} ]; do
        iexecc dd if=${write_file} of=${target_nvme} bs=${blk_unit_size_kb}K count=${io_count_at_once} seek=${current_seek_blk} skip=${current_skip_blk} conv=sparse,notrunc oflag=nocache status=none &
        remained_io_kb=$((${remained_io_kb}-${parallel_io_size_kb}))
        current_seek_blk=$((${current_seek_blk}+${io_count_at_once}))
        current_skip_blk=$((${current_skip_blk}+${io_count_at_once}))
    done
    local last_remaining_size=$((${remained_io_kb}/${blk_unit_size_kb}))
    iexecc dd if=${write_file} of=${target_nvme} bs=${blk_unit_size_kb}K count=${last_remaining_size} seek=${current_seek_blk} skip=${current_skip_blk} conv=sparse,notrunc oflag=nocache status=none &
    wait

    echo 3 > /proc/sys/vm/drop_caches
    sleep 4
    print_info "Data write has been finished!"

    return 0
}

verify_data()
{
    local volNum=$1
    local volName="vol$1"

    #blk_offset=$2
    #io_blk_cnt=$3
    #blk_unit_size_kb=$4

    blk_offset=${blk_offset}
    io_blk_cnt=${io_blk_cnt}
    blk_unit_size_kb=${blk_size_kb}

    file_postfix=".dat"
    write_file="write_"${volName}${file_postfix}
    read_file="read_"${volName}${file_postfix}
    cmp_file="cmp_"${volName}${file_postfix}

    print_info "Starting to load written data for data verification..."
    iexecc rm -rf ${read_file} ${cmp_file}
    sleep 1

    target_nvme=`sudo nvme list | grep -E 'SPDK|POS|pos' | awk '{print $1}' | head -16 | sed -n ''${volNum}',1p'`
    if [[ "${target_nvme}" == "" ]] || ! ls ${target_nvme} > /dev/null ; then
        print_result "NVMe drive is not found..." 1
        return 1
    fi

    print_info "Read data: target=${target_nvme}, blk offset=${blk_offset}, blk cnt=${io_blk_cnt}, blk size=${blk_unit_size_kb}KB"

    local remained_io_kb=$((${io_blk_cnt}*${blk_unit_size_kb}))
    local parallel_io_size_kb=$((128*1024)) # 128 (MB) * 1024
    local io_count_at_once=$((${parallel_io_size_kb}/${blk_unit_size_kb}))
    local current_seek_blk=0
    local current_skip_blk=${blk_offset}
    local amount_once_kb=${blk_unit_size_kb}

    while [ ${remained_io_kb} -gt ${parallel_io_size_kb} ]; do
        iexecc dd if=${target_nvme} of=${read_file} bs=${blk_unit_size_kb}K count=${io_count_at_once} seek=${current_seek_blk} skip=${current_skip_blk} conv=sparse,notrunc oflag=nocache status=none &
        remained_io_kb=$((${remained_io_kb}-${parallel_io_size_kb}))
        current_seek_blk=$((${current_seek_blk}+${io_count_at_once}))
        current_skip_blk=$((${current_skip_blk}+${io_count_at_once}))
    done
    local last_remaining_size=$((${remained_io_kb}/${blk_unit_size_kb}))
    iexecc dd if=${target_nvme} of=${read_file} bs=${blk_unit_size_kb}K count=${last_remaining_size} seek=${current_seek_blk} skip=${current_skip_blk} conv=sparse,notrunc oflag=nocache status=none &
    wait

    print_info "Verifying..."
    cmp_file="cmpfile"${file_postfix}
    parallel_dd ${write_file} ${cmp_file} ${blk_unit_size_kb} ${io_blk_cnt} 0 0

    iexecc cmp -b ${cmp_file} ${read_file} -n $((${blk_unit_size_kb}*${io_blk_cnt}*1024))
    res=$?
    if [ ${res} -eq 0 ]; then
        print_result "Verified data successfully" 0
        iexecc rm -rf ${cmp_file} ${read_file}
        return 0
    else
        iexecc hexdump ${cmp_file} > hexdump.${cmp_file}
        iexecc hexdump ${read_file} > hexdump.${read_file}
        print_result "Data miscompare detected...Please check out hexdump.${cmp_file} & hexdump.${read_file} in the current directory. Exit..." 1
        echo -e "See log file: ${logfile}"
        return 1
    fi
}

write_and_verify()
{
    local volNum=$1
    local volName="vol$1"

    write_data ${volNum}
    res=$?
    if [ $res -ne 0 ]; then
        return 1
    else
        verify_data ${volNum}
    fi

    return $?
}

EXPECT_PASS()
{
    local name=$1
    local result=$2

    if [ $result -eq 0 ]; then
        print_result "\"${name}\" passed as expected" 0
    else
        print_result "\"${name}\" failed as unexpected" 1
        # pause
        kill_pos
        exit 1
    fi
}

EXPECT_FAIL()
{
    local name=$1
    local result=$2

    if [ $result -ne 0 ]; then
        print_result "\"${name}\" failed as expected" 0
    else
        print_result "\"${name}\" passed as unexpected" 1
        kill_pos
        exit 1
    fi
}

network_module_check()
{
    texecc ./test/regression/network_module_check.sh
}

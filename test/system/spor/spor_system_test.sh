#!/bin/bash

# README
#
# require to modify below before running....
# modified:   ../test/system/network/network_config.sh

# change working directory to where script exists
cd $(dirname $0)

print_help()
{
cat << EOF
POR quick test script

Synopsis
    ./spor_quick_test.sh [OPTION]

Prerequisite
    1. please make sure that file below is properly configured according to your env.
        {IBOFOS_ROOT}/test/system/network/network_config.sh
    2. please make sure that ibofos binary exists on top of ${IBOFOS_ROOT}
    3. please configure your ip address, volume size, etc. propertly by editing npor_quick_test.sh

Description
    -x [execution mode]
        0: loop-back test (Used when initiator & target machine are identical) (default)
        1: normal test over fabric
        2: script verification
    -t [test_option]
        0: NPOR test with IO & data verification (default) 
        1: NPOR test with IO
        2: NPOR test only
        3: SPOR test with IO & data verification (NEW)
    -d [device type]
        0: iBoFOS bdev (default) 
        1: iBOFOS bdev under mock drives (can't support data verification)
        2: SPDK malloc bdev (can't support data verification)
    -i [test_iteration_cnt]
        Repeat test sequence n times according to the given value
        Default setting value is 10
    -m
        Manual mode for ibofos start. You should start ibofos application by yourself according to follow the indication.
        You can use this option for debugging purpose.
    -h 
        Show script usage

Default configuration (if specific option not given)
    ./npor_quick_test.sh -x 0 -t 0 -d 0 -i 3 

EOF
    exit 0
}

#---------------------------------
# Default configuration (if specific option not given)
exec_mode=1
test_option=3
test_iter_cnt=4
device_type=0
manual_ibofos_run_mode=0
with_wdt_daemon=0
do_io=1
verify_data_en=1
#---------------------------------
# manual configuration (edit below according to yours)
ibof_phy_volume_size_mb=102400
test_volume_size_mb=102400
max_io_range_mb=1024 #128
#ibof_dir="/home/seb/workspace/ibofos"
ibof_dir=$(readlink -f $(dirname $0))/../../..

#cwd=${ibof_dir}
cwd="/root/ibofos"
target_ip="10.1.1.15"
target_fabric_ip="172.16.1.1"
#target_ip="10.1.11.10"
#target_fabric_ip="10.100.11.10"
trtype=tcp
port="1158"
target_dev_list=("unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3")
#target_dev_list=("unvme-ns-0" "unvme-ns-1" "unvme-ns-2" "unvme-ns-3" "unvme-ns-4" "unvme-ns-5" "unvme-ns-6" "unvme-ns-7")
#target_dev_list=("/dev/sdb1" "/dev/sdb2" "/dev/sdb3")
target_spare_dev="unvme-ns-4"
# target_spare_dev="unvme-ns-8"
#target_spare_dev="/dev/sdb4"
network_config_file="./test/system/network/network_config.sh"
nvme_cli="nvme"
uram_backup_dir="/etc/uram_backup"
#---------------------------------
# internal configuration
target_nvme=""
volname="Volume"
file_postfix=".npor.dat"
write_file="wdata${file_postfix}"
read_file="rdata${file_postfix}"
device_type_list=("iBoFOS bdev" "iBOFOS bdev under Mock drive")
exec_mode_list=("Loop-back test" "Normal test over fabric" "Script verification mode")
test_option_list=("NPOR test with data verification" "NPOR test with IO" "NPOR test only" "SPOR test with IO")
#io_size_kb_list=(4 8 16 32 64 128 256) #KB
io_size_kb_list=(64 128) # 128 256) #KB
spdk_rpc_script="./lib/spdk-19.10/scripts/rpc.py"
spdk_nvmf_tgt="./lib/spdk-19.10/app/nvmf_tgt/nvmf_tgt"
nss="nqn.2019-04.ibof:subsystem"
echo_slient=1
logfile="npor_test.log"
SUBSYSTEM_NUM=3
#---------------------------------
MBtoB=$((1024*1024))
GBtoB=$((1024*${MBtoB}))

texecc()
{
    case ${exec_mode} in
    0) # loop-back test
        cd ${cwd}
        $@
        ;;
    1) # over-fabric test
        sshpass -pibof ssh root@${target_ip} "cd ${cwd}; sudo $@"
        ;;
    2) # script verification test
        info "sshpass -p seb ssh -tt root@${target_ip} \"cd ${cwd}; sudo $@\""
        ;;
    esac
}

iexecc()
{
    case ${exec_mode} in
    0) # loop-back test
        $@
        ;;
    1) # over-fabric test
        sudo $@
        ;;
    2) # script verification test
        info sudo "$@"
        ;;
    esac
    
}

ibof_phy_volume_size_byte=2147483648 #$((${ibof_phy_volume_size_mb}*${MBtoB}))
test_volume_size_byte=1073741824 #$((${test_volume_size_mb}*${MBtoB}))
max_io_range_byte=1048576 #$((${max_io_range_mb}*${MBtoB}))
max_io_boundary_byte=$((${test_volume_size_byte} - ${max_io_range_byte}))
#---------------------------------
print_test_configuration()
{
    echo "------------------------------------------"
    echo "[SPOR Sequence Test Information]"
    echo "> Test environment:"
    echo "  - Target IP:            ${target_ip}"
    echo "  - Target ${trtype} IP:  ${target_fabric_ip}"
    echo "  - Port:                 ${port}"
    echo "  - Working directory:    ${cwd}"
    echo ""
    echo "> Test configuration:"
    echo "  - Device type:          ${device_type_list[${device_type}]}"
    echo "  - Execution mode:       ${exec_mode_list[${exec_mode}]}"
    echo "  - Manual mode:          ${manual_ibofos_run_mode}"
    echo "  - Test option:          ${test_option_list[${test_option}]}"
    echo "  - Test iteration:       ${test_iter_cnt} loop(s)"
    echo "  - iBoF volume size:     ${ibof_phy_volume_size_mb} (MB)"
    echo "  - Test volume range:    ${test_volume_size_mb} (MB)"
    echo "  - Max I/O range:        ${max_io_range_mb} (MB)"
    echo "  - # of Subsystems:      ${SUBSYSTEM_NUM}"
    echo ""
    echo "> Logging:"
    echo "  - File: ${logfile}@initiator, ibofos.log@target"
    echo ""
    echo "NOTE:"
    echo "  - Please make sure that manual configuration in this script has been properly edited"
    echo "  - Please make sure that fabric connection before running test..."
    echo "------------------------------------------"

    wait_any_keyboard_input;
}

check_env()
{
    if [ ! -f /usr/sbin/nvme ]; then
        sudo apt install -y nvme-cli &> /dev/null
        exit 2;
    fi

    if [ ! -f /usr/bin/sshpass ]; then
        sudo apt install -y sshpass &> /dev/null
    fi

    # cmd="grep 'seb ALL=NO' /etc/sudoers"
    # if ! texecc ${cmd}; then
    #     error "Please add seb account into /etc/sudoers @target in order not to ask its password"
    #     error "e.g. with root permission, type this: echo seb ALL=NOPASSWD: ALL >> /etc/sudoers"
    #     exit 2 
    # fi
}

setup_prerequisite()
{
    check_env;

    #iexecc chmod +x *.sh
    #iexecc chmod +x ${network_config_file}

    #texecc chmod +x *.sh
    #texecc chmod +x ${network_config_file}
    
    # rm all modules
    #rmmod nvme_rdma; rmmod nvme_fabrics; rmmod mlx5_ib; rmmod mlx5_core; rmmod rdma_ucm; rmmod rdma_cm; rmmod iw_cm; rmmod ib_uverbs; rmmod ib_umad; rmmod ib_cm; rmmod ib_core;

    if [ ${echo_slient} -eq 1 ] ; then
        rm -rf ${logfile}; 
        touch ${logfile};
    fi
    clean_up;
    #texecc ./script/setup_env.sh;

    #texecc ls /sys/class/infiniband/*/device/net >> ${logfile}
    #iexecc ls /sys/class/infiniband/*/device/net >> ${logfile}

    if [ ${trtype} == "rdma" ]; then
        echo -n "RDMA configuration for server..."
        texecc ${network_config_file} server >> ${logfile}
        wait
        echo "Done"

        echo -n "RDMA configuration for client..."
        iexecc ${network_config_file} client >> ${logfile}
        wait
        echo "Done"
    fi

    iexecc ifconfig >> ${logfile}
    texecc ifconfig >> ${logfile}
}

kill_ibofos()
{
    # kill ibofos if exists
    texecc ./test/script/kill_ibofos.sh &>> ${logfile}
    echo ""
}

clean_up()
{
    disconnect_nvmf_contollers;
    
    kill_ibofos;

    rm -rf *${file_postfix}
    rm -rf ${logfile}
    rm -rf ibofos.log

    texecc umount ${uram_backup_dir}
}

start_ibofos()
{
    # clean up obsolete temp files
    texecc rm -rf /dev/shm/ibof_nvmf_trace.pid*

    if [ ${manual_ibofos_run_mode} -eq 1 ]; then
        notice "Please start iBoFOS application now..."
        wait_any_keyboard_input
    else 
        notice "Starting ibofos..."
        texecc ./script/start_ibofos.sh
    fi

    if [ ${device_type} -eq 1 ]; then
        # mock drive
        iexecc sleep 3
    else
        iexecc sleep 5 # takes longer if ibofos accesses actual drives
    fi
    notice "Now ibofos is running..."
}

establish_nvmef_target()
{
    # https://spdk.io/doc/nvmf.html
    notice "Now establishing NVMe-oF target..."

    if [ ${trtype} == "rdma" ]; then
        create_trtype="RDMA"
    else
        create_trtype="TCP"
    fi

    for i in `seq 1 ${SUBSYSTEM_NUM}`
    do
        connect_port=${port}
        echo "Creating subsystem ${nss}$i"
        texecc ${spdk_rpc_script} nvmf_create_subsystem ${nss}$i -a -s IBOF0000000000000$i -d IBOF_VOL_$i #>> ${logfile}
    done

    for i in `seq 1 ${SUBSYSTEM_NUM}`
    do
        connect_port=${port}
        echo "Adding listener ${nss}$i, ip ${target_fabric_ip}, port ${connect_port}"
        texecc ${spdk_rpc_script} nvmf_subsystem_add_listener ${nss}$i -t ${create_trtype} -a ${target_fabric_ip} -s ${connect_port} #>> ${logfile}
    done

    notice "New NVMe subsystem accessiable via Fabric has been added successfully to target!"
}

discover_n_connect_nvme_from_initiator()
{
    notice "Discovering remote NVMe drives..."
    iexecc ${nvme_cli} discover -t ${trtype} -a ${target_fabric_ip} -s ${port}  #>> ${logfile};
    notice "Discovery has been finished!"
    
    notice "Connecting remote NVMe drives..."
    for i in `seq 1 ${SUBSYSTEM_NUM}`
    do
        iexecc ${nvme_cli} connect -t ${trtype} -n ${nss}$i -a ${target_fabric_ip} -s ${port}  #>> ${logfile};
        target_nvme=`sudo nvme list | grep -E 'SPDK|IBOF|iBoF' | awk '{print $1}' | head -16 | sed -n ''${i}',1p'`

        if [ ${exec_mode} -ne 2 ] && [[ "${target_nvme}" == "" ]] || ! ls ${target_nvme} > /dev/null ; then
            error "NVMe drive is not found..."
            exit 2
        fi

        notice "Remote NVMe drive (${target_nvme}) have been connected via NVMe-oF!"
    done
}

disconnect_nvmf_contollers()
{
    for i in `seq 1 ${SUBSYSTEM_NUM}`
    do
        iexecc ${nvme_cli} disconnect -n ${nss}$i #>> ${logfile} 
    done
    notice "Remote NVMe drive has been disconnected..."
}

prepare_write_file()
{
    if [ ${do_io} -eq 0  ]; then
        return;
    fi

    iexecc touch ${write_file}

    parallel_dd /dev/urandom ${write_file} 1024 ${max_io_range_mb} 0 0
    info "Test file (${write_file}) has been prepared..."
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
        local curr_seek_blk_offset=${seek_blk_offset}
        local curr_skip_blk_offset=${skip_blk_offset}
        
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
        iexecc dd if=${input_file} of=${output_file} bs=${blk_size_kb}K count=${blk_count} seek=${seek_blk_offset} skip=${skip_blk_offset} conv=sparse,notrunc oflag=nocache status=none &
        wait
    fi
}

wait_any_keyboard_input()
{
    echo "Press enter to continue..."
    read -rsn1 # wait for any key press
}

check_result_err_from_logfile()
{
    result=`tail -1 ${logfile}`
    if [[ ${result} =~ "-1" ]]; then
        error "Failed the command execution...Last error log is:" 
        error ${result}
        exit 2
    fi
}

write_pattern()
{
    if [ ${do_io} -eq 0  ]; then
        return;
    fi
    volNum=$1
    blk_offset=$2
    io_blk_cnt=$3
    blk_unit_size=$4

    notice "Write pattern data: Target=/dev/nvme${volNum}n1, blk offset=${blk_offset}, IO block cnt=${io_blk_cnt}, blk size= ${blk_unit_size}(KB)"
    parallel_dd ${write_file} "/dev/nvme${volNum}n1" ${blk_unit_size} ${io_blk_cnt} 0 ${blk_offset}

    iexecc echo 3 > /proc/sys/vm/drop_caches
    iexecc sleep 4
    notice "Data write has been finished!"
}

shutdown_ibofos()
{
    notice "Shutting down ibofos..."
    texecc ./bin/cli request exit_ibofos
    notice "Shutdown has been completed!"

    iexecc sleep 60

    texecc umount ${uram_backup_dir}
    texecc rm -rf ${uram_backup_dir}

    disconnect_nvmf_contollers;

    #kill_ibofos
    #notice "ibofos killed..."
    #texecc ./script/backup_latest_hugepages_for_uram.sh &>> ${logfile}
}

trigger_spo()
{
    notice "Trigger SPO..."

    kill_ibofos
    notice "ibofos killed..."

    if [ ${with_wdt_daemon} -eq 0 ]; then
        texecc ./script/backup_latest_hugepages_for_uram.sh &>> ${logfile}
        iexecc sleep 3
    fi

    disconnect_nvmf_contollers;
}

bringup_ibofos()
{
    local ibofos_volume_required=1
	#FIXME: need to create mfs, create_mfs=1 

    create_mfs=0
    if [ ! -z $1 ] && [ $1 == "create" ]; then
        create_mfs=1
    fi

    start_ibofos;

    texecc ${spdk_rpc_script} nvmf_create_transport -t ${trtype} -u 131072 -p 4 -c 0 #>> ${logfile}

    texecc ${spdk_rpc_script} bdev_malloc_create -b uram0 1024 512
    iexecc sleep 1

    texecc ./bin/cli request scan_dev >> ${logfile}
    texecc ./bin/cli request list_dev >> ${logfile}

    if [ ${create_mfs} -eq 1 ]; then
        info "Create array: ${target_dev_list}"
        texecc ./bin/cli request create_array -ft ${create_mfs} -b uram0 -d ${target_dev_list} -s ${target_spare_dev} >> ${logfile}
        #check_result_err_from_logfile
    else
        info "Load array: ${target_dev_list}"
        texecc ./bin/cli request load_array >> ${logfile}
        #check_result_err_from_logfile
    fi

    texecc ./bin/cli request mount_ibofos

    establish_nvmef_target;

    if [ ${ibofos_volume_required} -eq 1 ] && [ ${create_mfs} -eq 1 ]; then
        for i in `seq 1 ${SUBSYSTEM_NUM}`
        do
            info "Create volume....${volname}$i"
            texecc ./bin/cli request create_vol --name ${volname}$i --size ${ibof_phy_volume_size_byte} >> ${logfile};
            check_result_err_from_logfile
        done
    fi

    for i in `seq 1 ${SUBSYSTEM_NUM}`
    do
        info "Mount volume....${volname}$i"
        texecc ./bin/cli request mount_vol --name ${volname}$i >> ${logfile};
        check_result_err_from_logfile
    done

    discover_n_connect_nvme_from_initiator;
    
    notice "Bring-up iBoFOS done!"
}

verify_data()
{
    if [ ${verify_data_en} -eq 0 ]; then
        return
    fi

    volNum=$1
    blk_offset=$2
    io_blk_cnt=$3
    blk_unit_size=$4

    notice "Starting to load written data for data verification..."
    rm -rf ${read_file}
    iexecc sleep 2
    #wait_any_keyboard_input;

    notice "Read data: Target=/dev/nvme${volNum}n1, block offset=${blk_offset}, IO block cnt=${io_blk_cnt}, Blk size= ${blk_unit_size}(KB)"
    #iexecc dd if=${target_nvme} of=${read_file} bs=${blk_unit_size}K count=${io_blk_cnt} skip=${blk_offset} oflag=nocache status=none  >> ${logfile};
    parallel_dd "/dev/nvme${volNum}n1" ${read_file} ${blk_unit_size} ${io_blk_cnt} ${blk_offset} 0
    #wait_any_keyboard_input;

    notice "Verifying..."
    cmp_file="cmpfile"${file_postfix}
    rm -rf ${cmp_file}
    #iexecc dd if=${write_file} of=${cmp_file} bs=${blk_unit_size}K count=${io_blk_cnt} oflag=nocache status=none  >> ${logfile}
    parallel_dd ${write_file} ${cmp_file} ${blk_unit_size} ${io_blk_cnt} 0 0
    
    iexecc cmp -b ${cmp_file} ${read_file} -n $((${blk_unit_size}*${io_blk_cnt}*1024))
    res=$?
    if [ ${res} -eq 0 ]; then
        notice "No data mismatch detected."
        rm -rf ${cmp_file} ${read_file}
    else
        hexdump ${cmp_file} > hexdump.${cmp_file}
        hexdump ${read_file} > hexdump.${read_file}
        error "Data miscompare detected...Please check out hexdump.${cmp_file} & hexdump.${read_file} in the current directory. Exit..."
        error "See log file: ${logfile}"
        exit 2
    fi
}

npor=1
run_iteration()
{
    echo ""
    curr_iter=1
    while [ ${curr_iter} -le ${test_iter_cnt} ]; do
        echo -e "\n\033[1;32mStaring new SPOR iteration...[${curr_iter}/${test_iter_cnt}]..................................\033[0m"

        local max_idx_num_of_list=$((${#io_size_kb_list[@]} - 1))
        local idx=`shuf -i 0-${max_idx_num_of_list} -n 1`
        local blk_size_kb=${io_size_kb_list[${idx}]}

        local curr_max_io_boundary_blk=$((${max_io_boundary_byte}/1024/${blk_size_kb}))
        local blk_offset=`shuf -i 1-$((${curr_max_io_boundary_blk}-2)) -n 1`
        local curr_max_io_range_blk=$((${max_io_range_byte}/1024/${blk_size_kb}))
        #local io_blk_cnt=`shuf -i 1-$((${curr_max_io_range_blk}-2)) -n 1`
	local io_blk_cnt=10

        for i in `seq 1 ${SUBSYSTEM_NUM}`
        do
            local volNum=$(($i-1))
            write_pattern ${volNum} ${blk_offset} ${io_blk_cnt} ${blk_size_kb}
        done

	if [ $npor -eq 0 ]; then
		notice "******************* trigger spo *********************"
        	trigger_spo
		npor=1
	else
		notice "******************* trigger npo *********************"
		shutdown_ibofos
		npor=0
	fi

        bringup_ibofos 0;

        for i in `seq 1 ${SUBSYSTEM_NUM}`
        do
            local volNum=$(($i-1))
            verify_data ${volNum} ${blk_offset} ${io_blk_cnt} ${blk_size_kb}
            iexecc sleep 1
        done

        ((curr_iter++))
    done
}

date=
get_date()
{
    date=`date '+%Y-%m-%d %H:%M:%S'`
}

info()
{
    get_date;
    echo -e "${date} [Info]   $@";
    echo -e "${date} [Info]   $@" >> ${logfile}
}

error()
{
    get_date;
    echo -e "\033[1;33m${date} [Error] $@ \033[0m" 1>&2;
    echo -e "${date} [Error] $@" >> ${logfile}
}

notice()
{
    get_date;
    echo -e "\033[1;36m${date} [Notice] $@ \033[0m" 1>&2;
    echo -e "${date} [Notice] $@" >> ${logfile}

}

check_permission()
{
    if [ $EUID -ne 0 ]; then
        echo "Error: This script must be run as root permission.."
        exit 0;
    fi
}

check_permission

while getopts "t:i:hx:d:m" opt
do
    case "$opt" in
        t) test_option="$OPTARG"
            if [ ${test_option} -eq 0 || ${test_option} -eq 3 ]; then
                verify_data_en=1
            fi
            if [ ${test_option} -eq 2 ]; then
                do_io=0
            fi
            ;;
        i) test_iter_cnt="$OPTARG"
            ;;
        x) exec_mode="$OPTARG"
            ;;
        d) device_type="$OPTARG"
            ;;
        m) manual_ibofos_run_mode=1
            ;;
        # v) out=""; echo_slient=0
        #     ;;
        h) print_help
            ;;
        ?) exit 2
            ;;
    esac
done

#------------------------------------
print_test_configuration
setup_prerequisite

bringup_ibofos create

prepare_write_file

run_iteration

clean_up

echo -e "\n\033[1;32m SPOR test has been successfully finished! Congrats! \033[0m"

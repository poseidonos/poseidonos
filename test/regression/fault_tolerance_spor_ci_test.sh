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
fault-tolerance test with spor script for ci

Synopsis
    ./fault_tolerance_spor_ci_test.sh [OPTION]

Prerequisite
    1. please make sure that file below is properly configured according to your env.
        {IBOFOS_ROOT}/test/system/network/network_config.sh
    2. please make sure that poseidonos binary exists on top of ${IBOFOS_ROOT}
    3. please configure your ip address, volume size, etc. propertly by editing fault_tolerance_test.sh

Description
    -i [test_iteration_cnt]
        Repeat test sequence n times according to the given value
        Default setting value is 10
    -m
        Manual mode for poseidonos start. You should start poseidonos application by yourself according to follow the indication.
        You can use this option for debugging purpose.
    -f [target_fabric_ip]
    -s [volume_size_in_mb]
        Volume size in mb to create for test
    -h 
        Show script usage

Default configuration (if specific option not given)
    ./fault_tolerance_spor_ci_test.sh -i 10 -f ${fabric_ip}

EOF
    exit 0
}

#---------------------------------
# Default configuration (if specific option not given)
manual_ibofos_run_mode=0
#---------------------------------
# manual configuration (edit below according to yours)
ibof_phy_volume_size_mb=102400
test_volume_size_mb=102400
max_io_range_mb=512
dummy_size_mb=$((${max_io_range_mb}*2))
cwd=`pwd`
target_fabric_ip="10.100.1.25"
trtype=tcp
port="1158"
target_dev_list="unvme-ns-0,unvme-ns-1,unvme-ns-2"
detach_dev="unvme-ns-0"
target_spare_dev="unvme-ns-3"
nvme_cli="nvme"
root_dir="../../"
ibof_cli="${root_dir}bin/poseidonos-cli "
network_config_file="${root_dir}test/system/network/network_config.sh"
total_iter=3
array_name="POSArray"
#---------------------------------
# internal configuration
target_nvme=""
volname="Volume0"
file_postfix=".dat"
write_file="wdata.tmp"
read_file="read"${file_postfix}
cmp_file="cmp"${file_postfix}
device_type_list=("PoseidonOS bdev" "iBOFOS bdev under Mock drive")
io_size_kb_list=(64 128 256) #KB
spdk_rpc_script="${root_dir}lib/spdk/scripts/rpc.py"
spdk_nvmf_tgt="${root_dir}lib/spdk/app/nvmf_tgt/nvmf_tgt"
nss="nqn.2019-04.pos:subsystem1"
echo_slient=1
logfile="ft_test.log"
#---------------------------------
MBtoB=$((1024*1024))
GBtoB=$((1024*${MBtoB}))

ibof_phy_volume_size_byte=$((${ibof_phy_volume_size_mb}*${MBtoB}))
test_volume_size_byte=$((${test_volume_size_mb}*${MBtoB}))
max_io_range_byte=$((${max_io_range_mb}*${MBtoB}))
max_io_boundary_byte=$((${test_volume_size_byte} - ${max_io_range_byte}))
#---------------------------------
print_test_configuration()
{
    echo "------------------------------------------"
    echo "[Fault-Tolerance Test With SPOR Information]"
    echo "> Test environment:"
    echo "  - Target ${trtype} IP:  ${target_fabric_ip}"
    echo "  - Port:                 ${port}"
    echo "  - Working directory:    ${cwd}"
    echo ""
    echo "> Test configuration:"
    echo "  - Manual mode:          ${manual_ibofos_run_mode}"
    echo "  - iBoF volume size:     ${ibof_phy_volume_size_mb} (MB)"
    echo "  - Test volume range:    ${test_volume_size_mb} (MB)"
    echo "  - Max I/O range:        ${max_io_range_mb} (MB)"
    echo "  - Test Iteration:       ${total_iter}"
    echo ""
    echo "> Logging:"
    echo "  - File: ${logfile}@initiator, pos.log@target"
    echo ""
    echo "NOTE:"
    echo "  - Please make sure that manual configuration in this script has been properly edited"
    echo "  - Please make sure that fabric connection before running test..."
    echo "------------------------------------------"

}

network_module_check()
{
    ${root_dir}/test/regression/network_module_check.sh
}

setup_prerequisite()
{
    chmod +x *.sh
    chmod +x ${network_config_file}

    chmod +x *.sh
    chmod +x ${network_config_file}

    if [ ${echo_slient} -eq 1 ] ; then
        rm -rf ${logfile}; 
        touch ${logfile};
    fi
    clean_up;

    if [ ${trtype} == "rdma" ]; then
        echo -n "RDMA configuration for server..."
        ${network_config_file} server >> ${logfile}
        wait
        echo "Done"

        echo -n "RDMA configuration for client..."
        ${network_config_file} client >> ${logfile}
        wait
        echo "Done"
    fi

    ifconfig >> ${logfile}
    ifconfig >> ${logfile}
}

kill_poseidonos()
{
	pkill -9 poseidonos
    echo ""
}

clean_up()
{
    disconnect_nvmf_contollers;
    
    kill_poseidonos;

    rm -rf *${file_postfix}
    rm -rf ${logfile}
    rm -rf pos.log
}

start_ibofos()
{
	rm -rf /dev/shm/ibof_nvmf_trace.pid*
	echo "PoseidonOS starting..."

    if [ ${manual_ibofos_run_mode} -eq 1 ]; then
        notice "Please start PoseidonOS application now..."
        wait_any_keyboard_input
    else 
        notice "Starting poseidonos..."
        ${root_dir}/test/regression/start_poseidonos.sh
    fi

	result=`${root_dir}/bin/poseidonos-cli system info --json-res | jq '.Response.info.version' 2>/dev/null`
	while [ -z ${result} ];
	do
		echo "Wait PoseidonOS..."
		result=`${root_dir}/bin/poseidonos-cli system info --json-res | jq '.Response.info.version' 2>/dev/null`
		sleep 0.5
	done

    notice "Now poseidonos is running..."
}

establish_nvmef_target()
{
    # https://spdk.io/doc/nvmf.html
    notice "Now establishing NVMe-oF target..."

    if [ ${trtype} == "rdma" ]; then
        create_trtype="RDMA"
        ${spdk_rpc_script} nvmf_create_transport -t ${create_trtype} -u 131072 #>> ${logfile}
    else
        create_trtype="TCP"
        ${spdk_rpc_script} nvmf_create_transport -t ${create_trtype} -b 64 -n 4096 #>> ${logfile}
    fi

    
    ${spdk_rpc_script} nvmf_subsystem_add_listener ${nss} -t ${trtype} -a ${target_fabric_ip} -s ${port} #>> ${logfile}

    notice "New NVMe subsystem accessiable via Fabric has been added successfully to target!"
}

discover_n_connect_nvme_from_initiator()
{
    notice "Discovering remote NVMe drives..."
    ${nvme_cli} discover -t ${trtype} -a ${target_fabric_ip} -s ${port}  #>> ${logfile};
    notice "Discovery has been finished!"
    
    notice "Connecting remote NVMe drives..."
    ${nvme_cli} connect -t ${trtype} -n ${nss} -a ${target_fabric_ip} -s ${port}  #>> ${logfile};

    sleep 1

    target_nvme=`sudo nvme list | grep -E 'SPDK|POS|pos' | awk '{print $1}' | head -n 1`

    if [[ "${target_nvme}" == "" ]] || ! ls ${target_nvme} > /dev/null ; then
        error "NVMe drive is not found..."
        exit 2
    fi

    notice "Remote NVMe drive (${target_nvme}) have been connected via NVMe-oF!"
}

disconnect_nvmf_contollers()
{
    ${nvme_cli} disconnect -n ${nss} #>> ${logfile} 
    notice "Remote NVMe drive has been disconnected..."
}

prepare_write_file()
{

	if [ ! -s ${write_file} ]; then
    	touch ${write_file}
		parallel_dd /dev/urandom ${write_file} 1024 ${dummy_size_mb} 0 0
		info "Test file (${write_file}) has been prepared..."
	fi
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
            dd if=${input_file} of=${output_file} bs=${blk_size_kb}K count=${io_count_at_once} seek=${curr_seek_blk_offset} skip=${curr_skip_blk_offset} conv=sparse,notrunc oflag=nocache status=none &

            remaining_io_cnt_kb=$((${remaining_io_cnt_kb}-${parallel_io_size_kb}))
            curr_seek_blk_offset=$((${curr_seek_blk_offset}+${io_count_at_once}))
            curr_skip_blk_offset=$((${curr_skip_blk_offset}+${io_count_at_once}))
        done
        # handle last remaining portion
        local last_remaining_blk_cnt=$((${remaining_io_cnt_kb}/${blk_size_kb}))
        dd if=${input_file} of=${output_file} bs=${blk_size_kb}K count=${last_remaining_blk_cnt} seek=${curr_seek_blk_offset} skip=${curr_skip_blk_offset} conv=sparse,notrunc oflag=nocache status=none &
        wait
    else
        dd if=${input_file} of=${output_file} bs=${blk_size_kb}K count=${blk_count} seek=${seek_blk_offset} skip=${skip_blk_offset} conv=sparse,notrunc oflag=nocache status=none &
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

    local blk_offset=$1
    local io_blk_cnt=$2
    local blk_unit_size=$3

    notice "Write pattern data: blk offset=${blk_offset}, IO block cnt=${io_blk_cnt}, blk size=${blk_unit_size}(KB)"
    parallel_dd ${write_file} ${target_nvme} ${blk_unit_size} ${io_blk_cnt} 0 ${blk_offset}

    echo 3 > /proc/sys/vm/drop_caches
    notice "Data write has been finished!"
}

shutdown_poseidonos()
{
    notice "Shutting down poseidonos..."
	${ibof_cli} array unmount --array-name $array_name --force
	${ibof_cli} system stop --force
    notice "Shutdown has been completed!"

    disconnect_nvmf_contollers;

    notice "poseidonos is stopped"
    wait_ibofos
}

wait_ibofos()
{
    ps -C poseidonos > /dev/null
    while [[ ${?} == 0 ]]
    do
        sleep 0.5
        ps -C poseidonos > /dev/null
    done
}

bringup_ibofos()
{
    local ibofos_volume_required=1

    create_array=0
    if [ ! -z $1 ] && [ $1 == "create" ]; then
        create_array=1
    fi

    start_ibofos;
    disconnect_nvmf_contollers

    ${spdk_rpc_script} nvmf_create_subsystem ${nss} -a -s POS00000000000001  -d POS_VOLUME #>> ${logfile}
    ${spdk_rpc_script} bdev_malloc_create -b uram0 1024 512

    ${ibof_cli} device scan >> ${logfile}
    ${ibof_cli} device list >> ${logfile}

	if [ $create_array -eq 1 ]; then
        ${root_dir}bin/poseidonos-cli devel resetmbr
		info "Target device list=${target_dev_list}"
		${ibof_cli} array create -b uram0 -d ${target_dev_list} --array-name $array_name
	fi
	
	${ibof_cli} array mount --array-name $array_name

    if [ ${ibofos_volume_required} -eq 1 ] && [ ${create_array} -eq 1 ]; then
        info "Create volume....${volname}"
        ${ibof_cli} volume create --volume-name ${volname} --size ${ibof_phy_volume_size_byte}B --array-name $array_name >> ${logfile};
    fi

    if [ ${ibofos_volume_required} -eq 1 ]; then
        info "Mount volume....${volname}"
        ${ibof_cli} volume mount --volume-name ${volname} --array-name $array_name >> ${logfile};
    fi
    
    establish_nvmef_target;
    discover_n_connect_nvme_from_initiator;
    
    notice "Bring-up PoseidonOS done!"
}

verify_data()
{

    local blk_offset=$1
    local io_blk_cnt=$2
    local blk_unit_size=$3

    notice "Starting to load written data for data verification..."
    rm -rf ${read_file}
    #wait_any_keyboard_input;

    notice "Read data: Target block offset=${blk_offset}, IO block cnt=${io_blk_cnt}, Blk size= ${blk_unit_size}(KB)"
    parallel_dd ${target_nvme} ${read_file} ${blk_unit_size} ${io_blk_cnt} ${blk_offset} 0
    #wait_any_keyboard_input;

    notice "Verifying..."
    rm -rf ${cmp_file}
    parallel_dd ${write_file} ${cmp_file} ${blk_unit_size} ${io_blk_cnt} 0 0
    
    cmp -b ${cmp_file} ${read_file} -n $((${blk_unit_size}*${io_blk_cnt}*1024))
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

detach_device()
{
	local dev_name=${detach_dev}
    ${root_dir}/test/script/detach_device.sh ${dev_name} 1
    sleep 0.1
    notice "${dev_name} is detached."
}

add_spare()
{
    notice "add spare device ${dev_name}"
	local dev_name=${target_spare_dev}
	${ibof_cli} array addspare --spare ${dev_name} --array-name $array_name
}

waiting_for_rebuild_complete()
{
	notice "waiting for rebuild complete"
	while :
	do
		state=$(${ibof_cli} array list --array-name $array_name --json-res | jq '.Response.result.data.state')
		if [ $state = "\"NORMAL\"" ]; then
			break;
		else
            rebuild_progress=$(${ibof_cli} array list --array-name $array_name --json-res | jq '.Response.result.data.rebuildingProgress')
            info "Rebuilding Progress [${rebuild_progress}]"
			sleep 3
		fi
	done

}

run_test()
{
    echo ""
	echo -e "\n\033[1;32mStarting..................................\033[0m"

	local max_idx_num_of_list=$((${#io_size_kb_list[@]} - 1))
	local idx=`shuf -i 0-${max_idx_num_of_list} -n 1`
	local blk_size_kb=${io_size_kb_list[${idx}]}
	local blk_size_byte=$((${blk_size_kb}*1024))
	local max_io_boundary_blk=$((${max_io_boundary_byte}/1024/${blk_size_kb}))
	local boundary_cnt=3
	local io_boundary_blk=$((${max_io_boundary_blk}/${boundary_cnt}))
	local max_io_range_blk=$((${max_io_range_byte}/1024/${blk_size_kb}))
	local dummy_range_blk=$((${dummy_size_mb}*1024/${blk_size_kb}))
	
	local blk_offset=()
	local io_blk_cnt=()
	
	local iter=0
	local cnt=2
	while [ ${iter} -le ${cnt} ]; do
		boundary_start=$((${iter}*${io_boundary_blk}+1))
		boundary_end=$((${boundary_start}+${io_boundary_blk}-${max_io_range_blk}-2))
		blk_offset[${iter}]=`shuf -i ${boundary_start}-${boundary_end} -n 1`
		io_blk_cnt[${iter}]=`shuf -i 1-$((${max_io_range_blk}-2)) -n 1`
		((iter++))
	done

	bringup_ibofos create
	write_pattern 0 ${dummy_range_blk} ${blk_size_kb} #dummy write
	write_pattern ${blk_offset[0]} ${io_blk_cnt[0]} ${blk_size_kb} # write #0
	detach_device
	write_pattern ${blk_offset[1]} ${io_blk_cnt[1]} ${blk_size_kb} # write #1
	shutdown_poseidonos
	bringup_ibofos 0
	verify_data ${blk_offset[0]} ${io_blk_cnt[0]} ${blk_size_kb} # read #0
	verify_data ${blk_offset[1]} ${io_blk_cnt[1]} ${blk_size_kb} # read #1

	add_spare &
	write_pattern ${blk_offset[2]} ${io_blk_cnt[2]} ${blk_size_kb} # write #2

	#rebuilding
	verify_data ${blk_offset[0]} ${io_blk_cnt[0]} ${blk_size_kb} 0 # read #0
    trigger_spor
    backup_nvram
    bringup_ibofos 0
	verify_data ${blk_offset[1]} ${io_blk_cnt[1]} ${blk_size_kb} 1 # read #1
	verify_data ${blk_offset[2]} ${io_blk_cnt[2]} ${blk_size_kb} 2 # read #2
	#rebuild done
	waiting_for_rebuild_complete
	verify_data ${blk_offset[0]} ${io_blk_cnt[0]} ${blk_size_kb} # read #0
	verify_data ${blk_offset[1]} ${io_blk_cnt[1]} ${blk_size_kb} # read #1
	verify_data ${blk_offset[2]} ${io_blk_cnt[2]} ${blk_size_kb} # read #2
}

trigger_spor()
{
    kill_poseidonos
}

backup_nvram()
{
    $root_dir/script/backup_latest_hugepages_for_uram.sh
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


run_iter()
{
	local curr_iter=1
	while [ ${curr_iter} -le ${total_iter} ]; do
		echo -e "\n\033[1;32mStarting new iteration...[${curr_iter}/${total_iter}]..................................\033[0m"
		run_test
		disconnect_nvmf_contollers;
		shutdown_poseidonos
		((curr_iter++))
	done
}

check_permission

while getopts "t:i:hx:d:m:f:i:s:" opt
do
    case "$opt" in
        m) manual_ibofos_run_mode=1
            ;;
        h) print_help
            ;;
        f) target_fabric_ip="$OPTARG"
            ;;
        i) total_iter="$OPTARG"
            ;;
        s) ibof_phy_volume_size_mb="$OPTARG"
            test_volume_size_mb="$OPTARG"
            ;;
        ?) exit 2
            ;;
    esac
done

#------------------------------------
print_test_configuration
network_module_check
setup_prerequisite
prepare_write_file

run_iter

clean_up

echo -e "\n\033[1;32m Fault-Tolerance test has been successfully finished! Congrats! \033[0m"

#!/bin/bash

# README
#
# require to modify below before running....
# modified:   ../test/system/network/network_config.sh

# change working directory to where script exists
IBOFOS_ROOT=$(readlink -f $(dirname $0))/../..
cd ${IBOFOS_ROOT}

print_help()
{
cat << EOF
Multi Array Bring up Script
Array CNT = 2

Usage :
	multi_array_bring_up.sh -f (fabric_ip)

EOF
    exit 0
}

# manual configuration (edit below according to yours)
exec_mode=0
ibof_phy_volume_size_mb=102400
test_volume_size_mb=102400
max_io_range_mb=1024 #128
cwd="/home/ibof/ibofos/script"
target_ip="127.0.0.1"
target_fabric_ip="10.100.4.3"
trtype=tcp
port="1158"
target_name_0="ARRAY0"
target_name_1="ARRAY1"
target_dev_list_0="unvme-ns-0,unvme-ns-1,unvme-ns-2"
target_dev_list_1="unvme-ns-3,unvme-ns-4,unvme-ns-5"
nvme_cli="nvme"
uram_backup_dir="/etc/uram_backup"
#---------------------------------
# internal configuration
target_nvme=""
volname="Volume0"
#io_size_kb_list=(4 8 16 32 64 128 256) #KB
io_size_kb_list=(64 128 256) #KB
spdk_rpc_script="${IBOFOS_ROOT}/lib/spdk/scripts/rpc.py"
spdk_nvmf_tgt="../lib/spdk/app/nvmf_tgt/nvmf_tgt"
nss="nqn.2019-04.ibof:subsystem1"
echo_slient=1
logfile="/var/log/ibofos/multi_array_bringup.log"
#---------------------------------
MBtoB=$((1024*1024))
GBtoB=$((1024*${MBtoB}))

texecc()
{
    case ${exec_mode} in
    0) # loop-back test
        $@
        ;;
    1) # over-fabric test
        sshpass -p seb ssh -tt root@${target_ip} "cd ${cwd}; sudo $@"
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

ibof_phy_volume_size_byte=$((${ibof_phy_volume_size_mb}*${MBtoB}))
test_volume_size_byte=$((${test_volume_size_mb}*${MBtoB}))
max_io_range_byte=$((${max_io_range_mb}*${MBtoB}))
max_io_boundary_byte=$((${test_volume_size_byte} - ${max_io_range_byte}))
#---------------------------------
network_module_check()
{
    texecc ${IBOFOS_ROOT}/test/regression/network_module_check.sh
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

    iexecc chmod +x *.sh

    texecc chmod +x *.sh
    
    # rm all modules
    #rmmod nvme_rdma; rmmod nvme_fabrics; rmmod mlx5_ib; rmmod mlx5_core; rmmod rdma_ucm; rmmod rdma_cm; rmmod iw_cm; rmmod ib_uverbs; rmmod ib_umad; rmmod ib_cm; rmmod ib_core;

    if [ ${echo_slient} -eq 1 ] ; then
        rm -rf ${logfile}; 
        touch ${logfile};
    fi
    #clean_up;

    texecc ls /sys/class/infiniband/*/device/net 2>/dev/null >> ${logfile}
    iexecc ls /sys/class/infiniband/*/device/net 2>/dev/null >> ${logfile}

    iexecc ifconfig >> ${logfile}
    texecc ifconfig >> ${logfile}

	echo "Setup Prerequisite Done"
}

check_stopped()
{
	result=`texecc "pgrep ibofos -c"`
	while [ $result -ne 0 ]
	do
		result=`texecc "pgrep ibofos -c"`
		sleep 0.5
	done
}

kill_ibofos()
{
    # kill ibofos if exists
    texecc ${IBOFOS_ROOT}/test/script/kill_ibofos.sh 2>> ${logfile}
	check_stopped

    echo "iBoFOS killed"
}

clean_up()
{
	echo "cleanup"

    disconnect_nvmf_contollers;
    
    kill_ibofos;
    rm -rf ${logfile}
    rm -rf ibofos.log

    umount ${uram_backup_dir}
}

start_ibofos()
{
	texecc rm -rf /dev/shm/ibof_nvmf_trace.pid*
	echo "iBoFOS starting..."

    if [ ${manual_ibofos_run_mode} -eq 1 ]; then
        notice "Please start iBoFOS application now..."
        #wait_any_keyboard_input
    else 
        notice "Starting ibofos..."
        texecc ${IBOFOS_ROOT}/test/regression/start_ibofos.sh
    fi

	result=`texecc "${IBOFOS_ROOT}/bin/cli request info --json" | jq '.Response.info.state' 2>/dev/null`
	while [ -z ${result} ] || [ ${result} != '"NOT_EXIST"' ];
	do
		echo "Wait iBoFOS..."
		result=`texecc "${IBOFOS_ROOT}/bin/cli request info --json" | jq '.Response.info.state' 2>/dev/null`
		echo $result
		sleep 0.5
	done

    notice "Now ibofos is running..."
}

establish_nvmef_target()
{
    # https://spdk.io/doc/nvmf.html
    notice "Now establishing NVMe-oF target..."

    if [ ${trtype} == "rdma" ]; then
        create_trtype="RDMA"
        texecc ${spdk_rpc_script} nvmf_create_transport -t ${create_trtype} -u 131072 #>> ${logfile}
    else
        create_trtype="TCP"
        texecc ${spdk_rpc_script} nvmf_create_transport -t ${create_trtype} -b 64 -n 4096 #>> ${logfile}
    fi

    texecc ${spdk_rpc_script} nvmf_subsystem_add_listener ${nss} -t ${trtype} -a ${target_fabric_ip} -s ${port} #>> ${logfile}

    notice "New NVMe subsystem accessiable via Fabric has been added successfully to target!"
}

discover_n_connect_nvme_from_initiator()
{
    notice "Discovering remote NVMe drives..."
    iexecc ${nvme_cli} discover -t ${trtype} -a ${target_fabric_ip} -s ${port}  #>> ${logfile};
    notice "Discovery has been finished!"
    
    notice "Connecting remote NVMe drives..."
    iexecc ${nvme_cli} connect -t ${trtype} -n ${nss} -a ${target_fabric_ip} -s ${port}  #>> ${logfile};
    echo `sudo nvme list | grep -E 'SPDK|IBOF|iBoF'`
    target_nvme=`sudo nvme list | grep -E 'SPDK|IBOF|iBoF' | awk '{print $1}' | head -1`
    echo $target_nvme
    #echo `sudo nvme list | grep -E 'SPDK|IBOF|iBoF' | awk '{print $1}' | head -n 1`
    #target_nvme=`sudo nvme list | grep -E 'SPDK|IBOF|iBoF' | awk '{print $1}' | head -n 1`

    if [ ${exec_mode} -ne 2 ] && [[ "${target_nvme}" == "" ]] || ! ls ${target_nvme} > /dev/null ; then
        error "NVMe drive is not found..."
        exit 2
    fi

    notice "Remote NVMe drive (${target_nvme}) have been connected via NVMe-oF!"
}

disconnect_nvmf_contollers()
{
    iexecc ${nvme_cli} disconnect -n ${nss} #>> ${logfile} 
    notice "Remote NVMe drive has been disconnected..."
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
shutdown_ibofos()
{
    notice "Shutting down ibofos..."
    texecc ${IBOFOS_ROOT}/bin/cli array unmount --name ${target_name_0}
    texecc ${IBOFOS_ROOT}/bin/cli array unmount --name ${target_name_1}
    texecc ${IBOFOS_ROOT}/bin/cli request exit_ibofos
    notice "Shutdown has been completed!"
	check_stopped

    disconnect_nvmf_contollers;

    #kill_ibofos
    #notice "ibofos killed..."
    #texecc ./backup_latest_hugepages_for_uram.sh &>> ${logfile}
    #iexecc sleep 3
}

bringup_ibofos()
{
    local ibofos_volume_required=1

    create_array=0
    if [ ! -z $1 ] && [ $1 == "create" ]; then
        create_array=1
    fi

    #start_ibofos;

    texecc ${spdk_rpc_script} nvmf_create_subsystem ${nss} -a -s IBOF00000000000001  -d IBOF_VOLUME #>> ${logfile}
    texecc ${spdk_rpc_script} bdev_malloc_create -b uram0 1024 512
    sleep 5
    texecc ${spdk_rpc_script} bdev_malloc_create -b uram1 1024 512

    texecc ${IBOFOS_ROOT}/bin/cli device scan >> ${logfile}
    texecc ${IBOFOS_ROOT}/bin/cli device list >> ${logfile}

	if [ $create_array -eq 1 ]; then
        texecc ${IBOFOS_ROOT}/bin/cli array reset
        info "Target device list=${target_dev_list_0} , ${target_dev_list_1}"
        texecc ${IBOFOS_ROOT}/bin/cli array create -b uram0 -d ${target_dev_list_0}  --name ${target_name_0}
        texecc ${IBOFOS_ROOT}/bin/cli array create -b uram1 -d ${target_dev_list_1}  --name ${target_name_1}
        #check_result_err_from_logfile
	fi
	
	texecc ${IBOFOS_ROOT}/bin/cli array mount --name ${target_name_0}
	texecc ${IBOFOS_ROOT}/bin/cli array mount --name ${target_name_1}


    if [ ${ibofos_volume_required} -eq 1 ] && [ ${create_array} -eq 1 ]; then
        info "Create volume....${volname}"
        texecc ${IBOFOS_ROOT}/bin/cli volume create --name ${volname} --size ${ibof_phy_volume_size_byte} --array ${target_name_0} >> ${logfile};
        texecc ${IBOFOS_ROOT}/bin/cli volume create --name ${volname} --size ${ibof_phy_volume_size_byte} --array ${target_name_1} >> ${logfile};
        #check_result_err_from_logfile
    fi

    if [ ${ibofos_volume_required} -eq 1 ]; then
        info "Mount volume....${volname}"
        texecc ${IBOFOS_ROOT}/bin/cli volume mount --name ${volname} --array ${target_name_0} >> ${logfile};
        texecc ${IBOFOS_ROOT}/bin/cli volume mount --name ${volname} --array ${target_name_1} >> ${logfile};
        #check_result_err_from_logfile
    fi
    
    establish_nvmef_target;
    discover_n_connect_nvme_from_initiator;
    
    notice "Bring-up iBoFOS done!"
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

while getopts "o:i:hx:d:f:t:" opt
do
    case "$opt" in
        f) target_fabric_ip="$OPTARG"
            ;;
        h) print_help
            ;;
        ?) exit 2
            ;;
    esac
done

#------------------------------------
network_module_check
setup_prerequisite
bringup_ibofos create

echo -e "\n\033[1;32m multi array bring up has been successfully finished! Congrats! \033[0m"


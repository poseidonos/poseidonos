#!/bin/bash

# README
#
# require to modify below before running....
# modified:   ../test/system/network/network_config.sh

# change working directory to where script exists
POS_ROOT=$(readlink -f $(dirname $0))/../..
cd ${POS_ROOT}

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
pos_phy_volume_size_mb=102400
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
target_nvme_0=""
target_nvme_1=""
volname="Volume0"
#io_size_kb_list=(4 8 16 32 64 128 256) #KB
io_size_kb_list=(64 128 256) #KB
spdk_rpc_script="${POS_ROOT}/lib/spdk/scripts/rpc.py"
spdk_nvmf_tgt="../lib/spdk/app/nvmf_tgt/nvmf_tgt"
nss1="nqn.2019-04.pos:subsystem1"
nss2="nqn.2019-04.pos:subsystem2"
echo_slient=1
logfile="/var/log/pos/multi_array_bringup.log"
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
        sshpass -p bamboo ssh -q -tt root@${target_ip} "cd ${cwd}; sudo $@"
        ;;
    2) # script verification test
        info "sshpass -p bamboo ssh -q -tt root@${target_ip} \"cd ${cwd}; sudo $@\""
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

pos_phy_volume_size_byte=$((${pos_phy_volume_size_mb}*${MBtoB}))
test_volume_size_byte=$((${test_volume_size_mb}*${MBtoB}))
max_io_range_byte=$((${max_io_range_mb}*${MBtoB}))
max_io_boundary_byte=$((${test_volume_size_byte} - ${max_io_range_byte}))
#---------------------------------
network_module_check()
{
    texecc ${POS_ROOT}/test/regression/network_module_check.sh
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
	result=`texecc "pgrep poseidonos -c"`
	while [ $result -ne 0 ]
	do
		result=`texecc "pgrep poseidonos -c"`
		sleep 0.5
	done
}

kill_poseidonos()
{
    # kill poseidonos if exists
    texecc ${POS_ROOT}/test/script/kill_poseidonos.sh 2>> ${logfile}
	check_stopped

    echo "poseidonOS killed"
}

clean_up()
{
	echo "cleanup"

    disconnect_nvmf_contollers;
    
    kill_poseidonos;
    rm -rf ${logfile}
    rm -rf pos.log

    umount ${uram_backup_dir}
}

start_poseidonos()
{
	texecc rm -rf /dev/shm/pos_nvmf_trace.pid*

    notice "Starting poseidonos..."
    texecc ${POS_ROOT}/test/regression/start_poseidonos.sh

	result=`texecc "${POS_ROOT}/bin/poseidonos-cli system info --json-res" | jq '.Response.data.version' 2>/dev/null`
	while [ -z ${result} ] || [ ${result} == '""' ];
	do
		echo "Wait iBoFOS..."
		result=`texecc "${POS_ROOT}/bin/poseidonos-cli system info --json-res" | jq '.Response.data.version' 2>/dev/null`
		echo $result
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
        texecc ${spdk_rpc_script} nvmf_create_transport -t ${create_trtype} -u 131072 #>> ${logfile}
    else
        create_trtype="TCP"
        texecc ${spdk_rpc_script} nvmf_create_transport -t ${create_trtype} -b 64 -n 4096 #>> ${logfile}
    fi

    texecc ${spdk_rpc_script} nvmf_subsystem_add_listener ${nss1} -t ${trtype} -a ${target_fabric_ip} -s ${port} #>> ${logfile}
    texecc ${spdk_rpc_script} nvmf_subsystem_add_listener ${nss2} -t ${trtype} -a ${target_fabric_ip} -s ${port} #>> ${logfile}

    notice "New NVMe subsystem accessiable via Fabric has been added successfully to target!"
}

discover_n_connect_nvme_from_initiator()
{
    notice "Discovering remote NVMe drives..."
    iexecc ${nvme_cli} discover -t ${trtype} -a ${target_fabric_ip} -s ${port}  #>> ${logfile};
    notice "Discovery has been finished!"
    
    notice "Connecting remote NVMe drives..."
    iexecc ${nvme_cli} connect -t ${trtype} -n ${nss1} -a ${target_fabric_ip} -s ${port}  #>> ${logfile};
    iexecc ${nvme_cli} connect -t ${trtype} -n ${nss2} -a ${target_fabric_ip} -s ${port}  #>> ${logfile};
    target_nvme_list=`sudo nvme list | grep -E 'SPDK|pos|POS' | awk '{print $1}'`
    echo $target_nvme_list
    target_nvme_0=`sudo nvme list | grep -E 'SPDK|pos|POS' | awk '{print $1}' | head -n 1`
    target_nvme_1=`sudo nvme list | grep -E 'SPDK|pos|POS' | awk '{print $1}' | tail -n 1`

    if [[ "${target_nvme_0}" == "" ]] || [[ "${target_nvme_1}" == "" ]] ||
        ! ls ${target_nvme_0} > /dev/null || ! ls ${target_nvme_1} > /dev/null ; then
        error "NVMe drive is not found..."
        exit 2
    fi

    notice "Remote NVMe drive (${target_nvme_0}, ${target_nvme_1}) have been connected via NVMe-oF!"
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
    notice "Shutting down poseidonos..."
    texecc ${POS_ROOT}/bin/poseidonos-cli array unmount --array-name ${target_name_0} --force
    texecc ${POS_ROOT}/bin/poseidonos-cli array unmount --array-name ${target_name_1} --force
    texecc ${POS_ROOT}/bin/poseidonos-cli system stop --force
    notice "Shutdown has been completed!"
	check_stopped

    disconnect_nvmf_contollers;

    #kill_poseidonos
    #notice "poseidonos killed..."
    #texecc ./backup_latest_hugepages_for_uram.sh &>> ${logfile}
    #iexecc sleep 3
}

bringup_poseidonos()
{
     local pos_volume_required=1

    create_array=0
    if [ ! -z $1 ] && [ $1 == "create" ]; then
        create_array=1
    fi

    #start_poseidonos;

    texecc ${spdk_rpc_script} nvmf_create_subsystem ${nss1} -a -s POS00000000000001  -d POS_VOLUME -m 256 #>> ${logfile}
    texecc ${spdk_rpc_script} nvmf_create_subsystem ${nss2} -a -s POS00000000000002  -d POS_VOLUME -m 256 #>> ${logfile}
    texecc ${spdk_rpc_script} bdev_malloc_create -b uram0 1024 512
    sleep 5
    texecc ${spdk_rpc_script} bdev_malloc_create -b uram1 1024 512

    texecc ${POS_ROOT}/bin/poseidonos-cli device scan >> ${logfile}
    texecc ${POS_ROOT}/bin/poseidonos-cli device list >> ${logfile}

	if [ $create_array -eq 1 ]; then
        texecc ${POS_ROOT}/bin/poseidonos-cli devel resetmbr
        info "Target device list=${target_dev_list_0} , ${target_dev_list_1}"
        texecc ${POS_ROOT}/bin/poseidonos-cli array create -b uram0 -d ${target_dev_list_0}  --array-name ${target_name_0}
        texecc ${POS_ROOT}/bin/poseidonos-cli array create -b uram1 -d ${target_dev_list_1}  --array-name ${target_name_1}
        #check_result_err_from_logfile
	fi
	
	texecc ${POS_ROOT}/bin/poseidonos-cli array mount --array-name ${target_name_0}
	texecc ${POS_ROOT}/bin/poseidonos-cli array mount --array-name ${target_name_1}


    if [ ${pos_volume_required} -eq 1 ] && [ ${create_array} -eq 1 ]; then
        info "Create volume....${volname}"
        texecc ${POS_ROOT}/bin/poseidonos-cli volume create --volume-name ${volname} --size ${pos_phy_volume_size_byte}B --array-name ${target_name_0} >> ${logfile};
        texecc ${POS_ROOT}/bin/poseidonos-cli volume create --volume-name ${volname} --size ${pos_phy_volume_size_byte}B --array-name ${target_name_1} >> ${logfile};
        #check_result_err_from_logfile
    fi

    if [ ${pos_volume_required} -eq 1 ]; then
        info "Mount volume....${volname}"
        texecc ${POS_ROOT}/bin/poseidonos-cli volume mount --volume-name ${volname} --array-name ${target_name_0} --subnqn ${nss1} >> ${logfile};
        texecc ${POS_ROOT}/bin/poseidonos-cli volume mount --volume-name ${volname} --array-name ${target_name_1} --subnqn ${nss2} >> ${logfile};
        #check_result_err_from_logfile
    fi
    
    establish_nvmef_target;
    #discover_n_connect_nvme_from_initiator;
    
    notice "Bring-up poseidonOS done!"
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
bringup_poseidonos create

echo -e "\n\033[1;32m multi array bring up has been finished! \033[0m"


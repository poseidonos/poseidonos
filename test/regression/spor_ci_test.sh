#!/bin/bash

# change working directory to where script exists
pwd=$(dirname $(realpath $0))
ibof_root=$(pwd)/../..
cd $pwd

print_help()
{
cat << EOF
SPOR regression test script

Synopsis
    ./spor_regression_test.sh [OPTION]

Prerequisite
    Build poseidonos with journaling

Description
    -t transport type, rdma or tcp
    -f target fabric ip
    -p port
    -l log directroy
    -q 
        Use quick mode to reduce test time
    --precommit-test 
    --postcommit-test
    -h
        show script usage

Default configuration (if specific option not given)
    ./spor_regression_test.sh -f 10.100.11.1 -t tcp -p 1158 -q --precommit-test 
EOF
    exit 0
}

#---------------------------------------------------
target_fabric_ip=`hostname -I | awk '{print $NF}'`
trtype=tcp
port="1158"
#---------------------------------------------------
nvme_cli="nvme"
nss="nqn.2019-04.pos:subsystem"
subsystem_num=6
uram_backup_dir="/tmp"
log_dir="$(pwd)/spor_log"
logfile="${log_dir}/spor_test.log"
test_mode="precommit"
#---------------------------------------------------

if [ ! -d ${log_dir} ] ; then
    mkdir ${log_dir}
fi
date=
get_date()
{
    date=`date '+%Y-%m-%d %H:%M:%S'`
}

RED="\033[1;31m"
GREEN="\033[0;32m"
RESET_COLOR="\033[0;0m"

info()
{
    get_date;
    echo "[${date}] [Info] $@" &>> ${logfile}
    echo -e "$GREEN[${date}] [Info]   $@$RESET_COLOR"
}

notice()
{
    get_date;
    echo "[${date}] [Notice] $@" &>> ${logfile}
    echo -e "$GREEN[${date}] [Notice]   $@$RESET_COLOR"
}

error()
{
    get_date;
    echo "[${date}] [Error] $@" &>> ${logfile}
    echo -e "$RED[${date}] [Error] $@ $RESET_COLOR" 1>&2
}

execc()
{
    get_date;
    echo "[${date}] [exec] $@" &>> ${logfile}
    echo -e "$GREEN[${date}] [exec]   $@$RESET_COLOR"
    $@
}

check_permission()
{
    if [ $EUID -ne 0 ]; then
        error "Error: This script must be run as root permission.."
        exit 1;
    fi
}

print_configuration()
{
    echo "------------------------------------------"
    echo "[SPOR Regression Test Information]"
    echo "  - Target IP:            ${target_fabric_ip}"
    echo "  - Transport type:       ${trtype}"
    echo "  - Port:                 ${port}"
    echo ""
    echo "------------------------------------------"

}

kill_ibofos()
{
    # kill poseidonos if exists
    execc ${ibof_root}/test/script/kill_poseidonos.sh
    echo ""
}

clean_up()
{
    for i in `seq 1 ${subsystem_num}`
    do
        execc ${nvme_cli} disconnect -n ${nss}$i #>> ${logfile} 
    done
    notice "Remote NVMe drive has been disconnected..."

	rm -rf ${uram_backup_dir}/*.uram.dat
    kill_ibofos
}

network_module_check()
{
    execc ${ibof_root}/test/regression/network_module_check.sh
}

run_test(){
    info "Start SPOR regression test"

    cd ${ibof_root}/test/system/spor

    if [ ${test_mode} = "precommit" ] ; then
        python3 run_all_tests.py -f ${target_fabric_ip} -l ${log_dir} -r 20 -q -s "SPOR_BASIC_[0,4].py, SPOR_VOLUME_BASIC_[4,6].py, SPOR_CHECK_POINT_4.py"
    elif [ ${test_mode} = "postcommit" ] ; then
        python3 run_all_tests.py -f ${target_fabric_ip} -l ${log_dir} -r 20 -q -s "SPOR_BASIC_[4,5].py, SPOR_MULTI_VOLUME_WRITE_2.py, SPOR_OVERWRITE_2.py, SPOR_CHECK_POINT*.py, SPOR_VOLUME_BASIC_4.py, SPOR_VOLUME_DELETE_5.py SPOR_MULTI_ARRAY_4.py"
    fi

    if [ $? -ne 0 ];then
        error "Test failed"
        exit 1
    fi 
}

while true; do
    case "$1" in
        -h | --help ) print_help; shift ;;
        -t | --trtype ) trtype="$2"; shift 2 ;;
        -p | --port ) port="$2"; shift 2 ;;
        -f | --target_fabric_ip ) target_fabric_ip="$2"; shift 2 ;;
        -l | --log_dir ) log_dir="$2"; shift 2 ;;
        -r | --run_time ) run_time="$2"; shift 2 ;;
        --precommit-test) test_mode="precommit"; shift ;;
        --postcommit-test) test_mode="postcommit"; shift ;;
        -- ) shift; break ;;
        * ) break ;;
    esac
done

# start test sequence::
info "Starting SPOR CI test..."
check_permission
print_configuration
clean_up
network_module_check 

run_test
clean_up

info "SPOR Test completed!"

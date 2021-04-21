#!/bin/bash

pwd=$(dirname $(realpath $0))
ibof_root=$(pwd)/../..
output_dir=$(pwd)/journal_manager_output
if [ ! -d ${output_dir} ] ; then
    mkdir ${output_dir}
fi

logfile="${output_dir}/regression_test.log"
journal_manager_root=${ibof_root}/src/journal_manager
spor_test_root=${ibof_root}/test/system/spor
#---------------------------------------------------
target_fabric_ip="10.100.11.23"
trtype=tcp
port="1158"
#---------------------------------------------------

RED="\033[1;31m"
LIGHT_GREEN="\033[0;32m"
GREEN="\033[1;32m"
RESET_COLOR="\033[0;0m"

date=
get_date()
{
    date=`date '+%Y-%m-%d %H:%M:%S'`
}

log_info()
{
    get_date;
    echo "[${date}] [Info] $@" &>> ${logfile}
    echo -e "${GREEN}[${date}] [Info] $@$RESET_COLOR"
}

log_error()
{
    get_date;
    echo "[${date}] [Error] $@" &>> ${logfile}
    echo -e "${RED}[${date}] [Error] $@ $RESET_COLOR" 1>&2
    exit 2;
}

execc()
{
    log_info "[exec] $@"
    $@
    ret=$?
    if [ $ret -ne 0 ]; then
        log_error "$@: Run Failed "
    fi
}
check_permission()
{
    if [ $EUID -ne 0 ]; then
        log_error "Error: This script must be run as root permission.."
    fi
}

print_configuration()
{
    echo "------------------------------------------"
    echo "[Journal Manager Regression Test Information]"
    echo "  - Target IP:            ${target_fabric_ip}"
    echo "  - Transport type:       ${trtype}"
    echo "  - Port:                 ${port}"
    echo "------------------------------------------"

    wait_any_keyboard_input
}

wait_any_keyboard_input()
{
    echo "Press enter to continue..."
    read -rsn1 # wait for any key press
}

build_ut()
{
    cd ${journal_manager_root}/unit_test
    if [ ! -f "journal_ut" ]; then
        log_info "[Build] Journal UT"
        make clean && make ut -j $(grep -c ^processor /proc/cpuinfo)
        ret=$?
        if [ $ret = 0 ]; then
			log_info "[Build] Journal UT.. Done"
		else
			log_error "[Build] Journal UT.. Error"
		fi
    fi
    cd -
}

build_ibofos()
{
    cd ${ibof_root}
    if [ ! -d "bin" ]; then
        log_info "[Build] poseidonos"
        ./configure
        make clean && make -j $(grep -c ^processor /proc/cpuinfo)
        ret=$?
        if [ $ret = 0 ]; then
			log_info "[Build] poseidonos.. Done"
		else
			log_error "[Build] poseidonos.. Error"
		fi
    fi
    cd -
}

run_ut()
{
    cd ${journal_manager_root}/unit_test
    log_info "Starting Journal Manager Unit test..."
    execc ./journal_ut
    log_info "Identifying memory leaks in unit test..."
    execc valgrind --leak-check=full --log-file=${output_dir}/journal_manager_memcheck.txt ./journal_ut 
    log_info "Generating a coverage report"
    execc lcov -t "result" -o ${output_dir}/journal_manager_gcov -c -d --rc lcov_branch_coverage=1 .
    execc genhtml --rc lcov_branch_coverage=1 -o ${output_dir}/gcov_html ${output_dir}/journal_manager_gcov
    log_info "Journal Manager Unit test has been successfully finished!"
    cd -
}

run_SPOR_test()
{
    log_info "Starting SPOR test..."

    cd ${spor_test_root}
    execc python3 run_all_tests.py -f ${target_fabric_ip} -t ${trtype} -p ${port} -l ${output_dir}

    cd -
    log_info "SPOR test has been successfully finished!"
}

log_info "Starting Journal regression test..."
check_permission
print_configuration
build_ut
run_ut
build_ibofos
run_SPOR_test
log_info "Journal regression test has been successfully finished! Congrats!"

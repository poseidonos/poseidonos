#!/bin/bash

IBOF_DIR_TOP="../../.."

target=$1

if [ $EUID -ne 0 ]; then
    echo "Error: This script must be run as root permission.."
    exit 0;
fi

if  [ $# -eq 0 ] || ( [ $target != "all" ] && [ $target != "mim" ] && [ $target != "mvm" ] && [ $target != "mfs" ] && [ $target != "msc" ] ) ; then
    echo "Error: incorrect input given...$1. Should one of 'all|mfs|mim|mvm|msc'"
    exit 0;
fi

export MFS_MAKE_FLAGS=-j

#------------------------------------------------------------

setup_env()
{
    (cd ${IBOF_DIR_TOP}/script && ./setup_env.sh )
	rm -rf /dev/shm/*
}
run_build()
{
    _target_ut=$1
	if [ ${_target_ut} == "mfs" ]; then
	    echo "Start to build PoseidonOS..."
		# PoseidonOS build required as of now
		(cd ${IBOF_DIR_TOP} && make clean > /dev/null 2> /dev/null && make ${MFS_MAKE_FLAGS} > /dev/null 2> /dev/null)
	fi
    echo "Start to build MFS..."
    (cd ../ && make ${MFS_MAKE_FLAGS} ut=${_target_ut} > /dev/null)
    echo "Build has been completed successfully"
    
}
check_target_built()
{
    pwd
    if [ ! -f ../mfs ]
    then
        echo "[Error] mfs binary build failed!"
        exit 1
    fi
}

clean_up_build()
{
    # clean up first
    (cd ../ && make clean)
    echo ""
}

run_ut_each()
{
    _target_ut=$1

    echo "Target UT=${_target_ut}"
    run_build ${_target_ut}
    check_target_built

    if [ ${_target_ut} == "mfs" ]; then
	    echo "Run test...${_target_ut}"
	    cd ..
        ./mfs
	    cd -
    else
	    echo "Run test w/ Valgrind...${_target_ut}"
        . ./run_mfs_with_valgrind.sh
    fi

    res=$?
    if [ $res -ne 0 ]; then
        echo "$1 UT failed..."
        exit 1
    fi

    echo "Clean up..."
    clean_up_build
}

run_ut_all()
{
    clean_up_build
    echo "Start to execute all UTs..."
    run_ut_each mvm
    run_ut_each mim
    run_ut_each msc
    run_ut_each mfs
}

# temporary change working directory where resides the script file
cd $(dirname $0)

setup_env

if [ ${target} == "all" ]; then
    run_ut_all
else
    run_ut_each ${target}
fi
echo "All unit test and integration test have been completed successfully!"

cd -

exit 0
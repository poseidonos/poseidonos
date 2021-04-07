#!/bin/sh

export MFS_MAKE_FLAGS=-j32
IBOF_DIR_TOP="../.."

UT_IN_SRC_FOLDER=1

# temporary change working directory where resides the script file
cd $(dirname $0)

error()
{
    echo "\033[1;33m[Error] $@ \033[0m" 1>&2;
}

notice()
{
    echo "\033[1;36m[Notice] $@ \033[0m" 1>&2;
}

# $1 = module ut directory name
# $2 = 1 if unit test is in src folder
build_test_ext_module_ut()
{
    if [ $2 -eq ${UT_IN_SRC_FOLDER} ]; then
        UT_DIR=${IBOF_DIR_TOP}/src/$1/unit_test
    else
        UT_DIR=${IBOF_DIR_TOP}/test/unittest/$1
    fi

    notice "============================================"
    notice "Start UT build test: [$1]"
    notice "============================================"
    (cd ${UT_DIR} && make clean && make ${MFS_MAKE_FLAGS})
    if [ $? -ne 0 ]; then
        error "[$1] UT Build failed...exit"
        exit 1
    fi

    notice "[OK]\n\n"
    wait 1
}

################

build_test_ext_module_ut "mapper"
build_test_ext_module_ut "allocator"
build_test_ext_module_ut "volume_manager"
build_test_ext_module_ut "journal_manager" ${UT_IN_SRC_FOLDER}
build_test_ext_module_ut "meta_file_intf"

echo -e "\n\033[1;32m All UT build sanity test for individual modules has been finished successfully!! \033[0m"

exit 0

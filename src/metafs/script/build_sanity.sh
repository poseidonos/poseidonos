#!/bin/sh

export MFS_MAKE_FLAGS=-j32
IBOF_DIR_TOP="../../.."

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

check_target_built()
{
if [ ! -f ../mfs ]
then
	error "Build failed! Exit..."
	exit 1
fi
}

clean_up_build()
{
# clean up first
(cd ../ && make clean)
(cd ${IBOF_DIR_TOP} && ./configure)
(cd ../ && make -C testdouble clean)
echo ""
}

# clean up first
clean_up_build

#########################################################
notice "Unit test build test"
notice "================================="
notice "* Unit test build for MIM block"
notice "================================="
(cd ../ && make ${MFS_MAKE_FLAGS} ut=mim)
check_target_built
clean_up_build

notice "================================="
notice "* Unit test build for MVM block"
notice "================================="
(cd ../ && make ${MFS_MAKE_FLAGS} ut=mvm)
check_target_built
clean_up_build

notice "================================="
notice "* Unit test build for MSC block"
notice "================================="

(cd ../ && make ${MFS_MAKE_FLAGS} ut=msc)
check_target_built
clean_up_build

# #########################################################
notice "================================="
notice "MFS product code build with Unit test option"
notice "================================="
(cd ${IBOF_DIR_TOP} && make clean > /dev/null 2> /dev/null && make ${MFS_MAKE_FLAGS})
(cd ../ && make ${MFS_MAKE_FLAGS} ut=mfs)
check_target_built
clean_up_build

notice "================================="
notice "Virtual filesystem build (fake MetaFs) with Unit test option"
notice "================================="
(cd ../ && make ${MFS_MAKE_FLAGS} ut=mfs vmfs=1)
check_target_built
clean_up_build

# #######################################################
notice "================================="
notice "MFS WBT API test option"
notice "================================="
(cd ${IBOF_DIR_TOP} && ./configure --with-wbt)
(cd ${IBOF_DIR_TOP} && make clean > /dev/null 2> /dev/null && make ${MFS_MAKE_FLAGS})
(cd ../ && make ${MFS_MAKE_FLAGS} ut=wbt)
check_target_built
clean_up_build

#########################################################
notice "================================="
notice "PoseidonOS binding build test"
notice "================================="
TARGET_OBJDIR=obj_output
export OBJDIR=${TARGET_OBJDIR}

mkdir -p ../${TARGET_OBJDIR}
(cd ../ && make ${MFS_MAKE_FLAGS} iboftarget && touch mfs)
check_target_built
clean_up_build

mkdir -p ../${TARGET_OBJDIR}
(cd ../ && make ${MFS_MAKE_FLAGS} iboftarget vmfs=1 && touch mfs)
check_target_built
clean_up_build

mkdir -p ../${TARGET_OBJDIR}
(cd ../ && make -C testdouble mock ${MFS_MAKE_FLAGS} && touch mfs)
check_target_built
clean_up_build

mkdir -p ../${TARGET_OBJDIR}
(cd ../ && make -C testdouble stub ${MFS_MAKE_FLAGS} && touch mfs)
check_target_built
clean_up_build

mkdir -p ../${TARGET_OBJDIR}
(cd ../ && make -C testdouble fake ${MFS_MAKE_FLAGS} && touch mfs)
check_target_built
clean_up_build

echo "--------------------------------------------------"
echo -n "\033[32m" && echo "Build sanity test has been finished successfully!" && echo -n "\033[0m"

exit 0

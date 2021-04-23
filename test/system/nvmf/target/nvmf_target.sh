#!/bin/bash
#
# nvmf_target.sh
#

rootdir=$(readlink -f $(dirname $0))/../../../../
echo $rootdir
echo $rootdir
echo $rootdir
echo $rootdir
SPDK_DIR=$rootdir/lib/spdk/

NVMF_TARGET_CONFIG=./nqn_ibof.conf

set -e

echo "config network information"
. $rootdir/test/system/network/network_config.sh server

echo "run ibof nvmf target"
case "$1" in
c)
	make -C c
	NVMF_TARGET_APP=./c/ibof_nvmf_tgt
	echo $NVMF_TARGET_APP -c $NVMF_TARGET_CONFIG
	$NVMF_TARGET_APP -c $NVMF_TARGET_CONFIG
	;;
cpp)
	make -C cpp
	NVMF_TARGET_APP=./cpp/nvmf_target_test
	echo $NVMF_TARGET_APP -c $NVMF_TARGET_CONFIG
	$NVMF_TARGET_APP -c $NVMF_TARGET_CONFIG
	;;
rpc)
	NVMF_TARGET_APP=./c/ibof_nvmf_tgt
	NR_SUBSYSTEM=2
	echo "run $NVMF_TARGET_APP with rpc commands"
	$NVMF_TARGET_APP &
	sleep 2
	$SPDK_DIR/scripts/rpc.py nvmf_create_transport -t TCP -n 4096 -b 64
	for i in `seq 1 $NR_SUBSYSTEM`
	do
		$SPDK_DIR/scripts/rpc.py bdev_null_create Null$i 1024 512
		$SPDK_DIR/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem$i -a -s POS0000000000000$i -d POS_VOLUME$i
		$SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_ns nqn.2019-04.pos:subsystem$i Null$i
		$SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem$i -t tcp -a $SERVER_IP -s 1158
	done
	$SPDK_DIR/scripts/rpc.py nvmf_get_subsystems
	;;
api)
	make -C cpp
	NVMF_TARGET_APP=./cpp/nvmf_target_test
	echo "run $NVMF_TARGET_APP with API"
	$NVMF_TARGET_APP api
	;;
gtest)
	make -C gtest
	NVMF_TARGET_APP=./gtest/nvmf_target_gtest
	echo "run $NVMF_TARGET_APP with gTest"
	$NVMF_TARGET_APP
	;;

kill)
	NVMF_TARGET_APP=./c/ibof_nvmf_tgt
	ps -ef | grep $NVMF_TARGET_APP | awk '{ print $2 }' | xargs kill -9 1>/dev/null
	;;
*)
	echo "Usage: nvmf_target.sh {cpp|api|rpc|gtest|c|kill}. Will choose cpp(NvmfTargetTest) with config file by default"
	make -C cpp
	NVMF_TARGET_APP=./cpp/nvmf_target_test
	echo $NVMF_TARGET_APP -c $NVMF_TARGET_CONFIG
	$NVMF_TARGET_APP -c $NVMF_TARGET_CONFIG
	;;
esac


exit 0

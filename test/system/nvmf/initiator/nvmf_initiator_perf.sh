#!/bin/bash
#
# nvmf_initiator_perf.sh
#

rootdir=$(readlink -f $(dirname $0))/../../../../
spdk_dir=$rootdir/lib/spdk

NVMF_TARGET_NQN=nqn.2019-04.pos:subsystem1
NVMF_TRANSPORT=TCP
#NVMF_TARGET_NQN=nqn.2019-04.pos:subsystem1
#NVMF_TRANSPORT=RDMA
NVMF_TARGET_PORT=1158

NVMF_INITIATOR_APP=$spdk_dir/examples/nvme/perf/perf

set -e

echo "config rdma nic"
. $rootdir/test/system/network/network_config.sh client
if [ $? != 0 ]; then
	echo "fail to configure RDMA NIC info, check network_config.sh"
	exit 1
fi

run_io(){
	log_normal "run perf"
	$NVMF_INITIATOR_APP -q 128 -o 10240 -s 512 -w rw -M 50 -t 10 -r "trtype:$NVMF_TRANSPORT adrfam:IPv4 traddr:$RDMA_SERVER_IP trsvcid:$NVMF_TARGET_PORT subnqn:$NVMF_TARGET_NQN"
	if [ $? = 0 ]; then
		log_normal "perf done - success"
	else
		log_error "perf done - failure"
	fi
}


echo "run nvmf initiator - perf "
run_io

#!/bin/bash
#
# nvmf_initiator_fio.sh
#

rootdir=$(readlink -f $(dirname $0))/../../../../
spdk_dir=$rootdir/lib/spdk
NVMF_INITIATOR_APP=$spdk_dir/examples/nvme/fio_plugin/fio_plugin

set -e

echo "config rdma nic"
. $rootdir/test/system/network/network_config.sh client
if [ $? != 0 ]; then
	echo "fail to configure RDMA NIC info, check network_config.sh"
	exit 1
fi

#NVMF_TRANSPORT=rdma
NVMF_TRANSPORT=tcp
NVMF_TARGET_IP=$RDMA_SERVER_IP
NVMF_TARGET_PORT=1158

echo \"$NVMF_FILENAME\"

run_io(){
	log_normal "run fio"
	fio --thread=1 --ioengine=$NVMF_INITIATOR_APP --group_reporting=1 --direct=1 --verify=md5 --time_based=1 --ramp_time=0 --runtime=3 --iodepth=128 -rw=randrw --bs=4k --numjobs=1 -name=fio_direct_write_test --filename="trtype=$NVMF_TRANSPORT adrfam=IPv4 traddr=$NVMF_TARGET_IP trsvcid=$NVMF_TARGET_PORT ns=1"
	if [ $? = 0 ]; then
		log_normal "fio done - success"
	else
		log_error "fio done - failure"
	fi
}


echo "run nvmf initiator - fio "
run_io

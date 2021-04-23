#!/bin/bash
#
# network_config.sh : network(rdma/tcp) server/client config
#
###################################################################################
#Note : below 5 variables need to be properly setted
#TRANSPORT : rdma or tcp
TRANSPORT=tcp

SERVER_NIC=ens192f0
SERVER_IP=10.100.11.20

CLIENT_NIC=ens192f0
CLIENT_IP=10.100.11.21

###################################################################################

set_red(){
	echo -e "\033[31m"
}
set_green(){
	echo -e "\033[32m"
}

set_white(){
	echo -e "\033[0m"
}

log_normal(){
	set_green && echo $1 && set_white
}

log_error(){
	set_red && echo $1 && set_white
}

if [ -z "$(lsmod | grep nvme_rdma)" ]; then
	modprobe ib_cm
	modprobe ib_core
	# Newer kernels do not have the ib_ucm module
	modprobe ib_ucm || true
	modprobe ib_umad
	modprobe ib_uverbs
	modprobe iw_cm
	modprobe rdma_cm
	modprobe rdma_ucm

	modprobe mlx5_core
	modprobe mlx5_ib
	modprobe nvme_rdma
	modprobe nvme_tcp
fi


if [ "$TRANSPORT" = 'rdma' ]; then
	if [ -z "$(which ib_write_bw)" ]; then
		echo "no RDMA test tool. run apt-get install -y ibverbs-utils perftest"
		exit 1
	fi
	MTU_SIZE=9000
elif [ "$TRANSPORT" = 'tcp' ]; then
	MTU_SIZE=9000
else
	MTU_SIZE=1500
fi


ibv_devinfo
case "$1" in
server)
	ifconfig $SERVER_NIC $SERVER_IP
	ifconfig $SERVER_NIC mtu $MTU_SIZE
	;;
client)
	ifconfig $CLIENT_NIC $CLIENT_IP
	ifconfig $CLIENT_NIC mtu $MTU_SIZE
	;;
*)
	exit 1
	;;
esac


#!/bin/bash
#
# network_test.sh : network server/client test
#
# note) network information needs to be properly configured prior to run the script


rootdir=$(readlink -f $(dirname $0))/../../../
set -e

case "$1" in
server)
	. $rootdir/test/system/network/network_config.sh server
	sleep 1
	if [ "$TRANSPORT" = 'rdma' ]; then
		ib_write_bw -d mlx5_0 -i 1 -M $MTU_SIZE -F --report_gbits
	elif [ "$TRANSPORT" = 'tcp' ]; then
		echo iperf -s -i 1 -M $MTU_SIZE -B $SERVER_IP -p 1158
		iperf -s -i 1 -M $MTU_SIZE -B $SERVER_IP -p 1158

	fi
	;;
client)
	. $rootdir/test/system/network/network_config.sh client
	sleep 1
	if [ "$TRANSPORT" = 'rdma' ]; then
		ib_write_bw -d mlx5_0 -i 1 -M $MTU_SIZE -F --report_gbits $SERVER_IP
	elif [ "$TRANSPORT" = 'tcp' ]; then
		echo iperf -c $SERVER_IP -M $MTU_SIZE -p 1158
		iperf -c $SERVER_IP -M $MTU_SIZE -p 1158
	fi
	;;

*)
	echo "Usage: network_test.sh {server|client}"
	exit 1
	;;
esac

exit 0


#!/bin/bash
#

#stop on first error.

set -e

# This IP should be 10.1.xx.xx, do not input 172.16.1.x
# The reason is that 172.16.1.x and below ip is used for message (netcat) operation to sync up between server and client.

SERVER_IP=10.1.4.15
CLIENT_IP=10.1.4.10
SCRIPT_PATH=../../../../script/


log_normal(){
    echo -e "\033[32m"$1"\033[0m"
}

log_error(){
    echo -e "\033[31m"$1"\033[0m"
}

TEST_NUM=10
SERVER_SIG=0xABCD
SIGNAL_PORT=1234

signal(){
    sleep 5
    echo "send"        
    echo "$SERVER_SIG""$1" | nc $2 $SIGNAL_PORT & pkill nc
}

wait_signal(){
    rm -rf temp.txt
    nc -l -p $SIGNAL_PORT > temp.txt
    RECV_SIG=`cat temp.txt`     
    echo "recv"        
    if [ $RECV_SIG != "$SERVER_SIG""$1" ];then
        log_error "SIGNATURE FAILED!!!!"
        exit 1
    fi              
}

case "$1" in
server)

echo "Wait Client ....."
wait_signal 0

for i in `seq 1 $TEST_NUM`
do
    #Wait client's bring up.

    log_normal "### Test"$i" ###"
    cd $SCRIPT_PATH
    ./setup_vpp.sh
    sleep 1

    cd -
    ./prepare_nvmf_target.sh
    
    sleep 2
    signal $i $CLIENT_IP
    wait_signal $i

    ./finalize_nvmf_target.sh
        
    cd $SCRIPT_PATH
    ./clear_vpp.sh
    sleep 1
    cd -

done
;;

client)

signal 0 $SERVER_IP

for i in `seq 1 $TEST_NUM`
do
    #Wait Server's 
    wait_signal $i
    ./test_connect_multiple.py 5

    sleep 10

    signal $i $SERVER_IP

done
;;
*)
   echo "Usage ./open_close_spdk_vpp_repeated.sh {server|client}"

esac   
exit 0



#!/bin/bash
#
# nvmf_initiator_nvme_cli.sh
#

rootdir=$(readlink -f $(dirname $0))/../../../../
spdk_dir=$rootdir/lib/spdk

TEST_NUM=1

NVMF_TARGET_NQN=nqn.2019-04.pos:subsystem1
NVMF_TARGET_PORT=1158
NVMF_INITIATOR_APP=nvme
SERVER_IP=172.16.1.1
NVMF_TRANSPORT=tcp

# Note: It is turned out that, 'discover' commands of nvme-cli working on kernel nvme-tcp driver to detect TCP transport. Thus, it is needed to use kernel v5.0 above for tcp transport
#NVMF_TRANSPORT=tcp
#NVMF_TRANSPORT=rdma

set -e
echo "config"$NVMF_TRNSPORT"nic"

# Note : If network configuration is necessary, please uncomment. 
#. $rootdir/test/system/network/network_config.sh client

if [ $? != 0 ]; then
    echo "fail to configure RDMA NIC info, check network_config.sh"
    exit 1
fi

log_normal(){
	echo -e "\033[32m"$1"\033[0m"
}

log_error(){
	echo -e "\033[31m"$1"\033[0m"
}	


discovery(){
    log_normal "discover"
    $NVMF_INITIATOR_APP discover -t $NVMF_TRANSPORT -a $SERVER_IP -s $NVMF_TARGET_PORT
    echo $NVMF_INITIATOR_APP discover -t $NVMF_TRANSPORT -a $SERVER_IP -s $NVMF_TARGET_PORT
    if [ $? = 0 ]; then
        log_normal "discover done - success"
    else
        log_error "discover done - failure"
    fi
}

connect(){
    log_normal "connect"
    $NVMF_INITIATOR_APP connect -t $NVMF_TRANSPORT -n "$NVMF_TARGET_NQN" -a $SERVER_IP -s $NVMF_TARGET_PORT
    echo $NVMF_INITIATOR_APP connect -t $NVMF_TRANSPORT -n "$NVMF_TARGET_NQN" -a $SERVER_IP -s $NVMF_TARGET_PORT
    if [ $? = 0 ]; then
        log_normal "connect done - success"
    else
        log_error "connect done - failure"
    fi
}

disconnect(){
    log_normal "disconnect"
    $NVMF_INITIATOR_APP disconnect -n "$NVMF_TARGET_NQN"
    echo $NVMF_INITIATOR_APP disconnect -n "$NVMF_TARGET_NQN"
    if [ $? = 0 ]; then
        log_normal "disconnect done - success"
    else
        log_error "disconnect done - failure"
    fi
}


connect_once(){
    echo "run nvmf initiator - nvme "
    discovery
    sleep 2
    disconnect
    sleep 2
    connect
    sleep 2
    disconnect
}

connect_multiple(){
    SET=$(seq 0 $TEST_NUM)
    for i in $SET
    do
	log_normal "### Test"$i" ###"
        connect_once
        sleep 2
    done
}

if [ $# -eq 0 ]; then
    connect_once
else
    TEST_NUM=`expr $1`
    connect_multiple
fi



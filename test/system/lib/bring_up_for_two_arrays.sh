#!/bin/bash
ROOT_DIR=$(readlink -f $(dirname $0))/../../../
SPDK_DIR=$ROOT_DIR/lib/spdk

PORT_COUNT=1
# Note: In case of tcp transport, network io irq can be manually controlled for better performance by changing SET_IRQ_AFFINITY=TRUE with given TARGET_NIC and NET_IRQ_CPULIST 
SET_IRQ_AFFINITY=FALSE
TARGET_NIC=enp59s0
NET_IRQ_CPULIST=29-41,42-55

DEFAULT_TRANSPORT=TCP
DEFAULT_TARGET_IP=10.100.11.1   # CI Server VM IP
DEFAULT_SUBSYSTEM_COUNT=1

############## ^^^ USER CONFIGURABLES ^^^ #################

RED_COLOR="\033[1;31m"
GREEN_COLOR="\033[0;32m"
RESET_COLOR="\033[0;0m"

log_normal(){
    echo -e $GREEN_COLOR$1$RESET_COLOR
}

log_error(){
    echo -e $RED_COLOR$1$RESET_COLOR
}

pos_bringup(){
    if [ "$TRANSPORT" == "TCP" ] || [ "$TRANSPORT" == "tcp" ]; then 
        sudo $ROOT_DIR/test/system/network/tcp_tune.sh max
    	if [ "$SET_IRQ_AFFINITY" == "TRUE" ] || [ "$SET_IRQ_AFFINITY" == "true" ];then
            sudo systemctl stop irqbalance.service
            sudo $ROOT_DIR/test/script/set_irq_affinity_cpulist.sh ${NET_IRQ_CPULIST} ${TARGET_NIC}
    	fi
        sudo $SPDK_DIR/scripts/rpc.py nvmf_create_transport -t $TRANSPORT -b 64 -n 2048
    else
        sudo $SPDK_DIR/scripts/rpc.py nvmf_create_transport -t $TRANSPORT -u 131072
    fi
    sudo $SPDK_DIR/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem1 -a -s POS00000000000001 -d POS_VOLUME_EXTENTION -m 256
    sudo $SPDK_DIR/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem2 -a -s POS00000000000002 -d POS_VOLUME_EXTENTION -m 256
    sudo $SPDK_DIR/scripts/rpc.py bdev_malloc_create -b uram0 1024 512
    sleep 5
    sudo $SPDK_DIR/scripts/rpc.py bdev_malloc_create -b uram1 1024 512
    sudo $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem1 -t $TRANSPORT -a $TARGET_IP -s 1158
    sudo $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem2 -t $TRANSPORT -a $TARGET_IP -s 1158
}

print_help()
{
    cat <<- END_OF_HELP

    usage: $(basename $0) [-h] [-c 0|1] [-t rdma|tcp] [-a xxx.xxx.xxx.xxx]

    Configures POS to be IO-Service-Ready.

    Options:
    -h: Print this help page
    -t: Transport protocol, Default: $DEFAULT_TRANSPORT
    -a: IP Address of target server, Default: $DEFAULT_TARGET_IP

END_OF_HELP
}

while getopts t:a:h ARG ; do
    case $ARG in
        t )
            TRANSPORT=$OPTARG
            ;;
        a )
            TARGET_IP=$OPTARG
            ;;
        h )
            print_help ;
            exit 0 
            ;;
        ? ) print_help >&2 ;
            exit 1
            ;;
    esac
done
shift $(($OPTIND - 1))

if [ -z $TRANSPORT ]; then
TRANSPORT=$DEFAULT_TRANSPORT
fi

if [ -z $TARGET_IP ]; then
TARGET_IP=$DEFAULT_TARGET_IP
fi
pos_bringup

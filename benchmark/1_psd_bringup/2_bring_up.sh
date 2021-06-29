#!/bin/bash
# Note : increase VOLUME_COUNT & SUBSYSTEM_COUNT will make multiple volumes and namespace (1:1)

source ../config.sh
ROOT_DIR=$(readlink -f $(dirname $0))/../../
SPDK_DIR=$ROOT_DIR/lib/spdk

PORT_COUNT=1
# Note: In case of tcp transport, network io irq can be manually controlled for better performance by issueing an option, "-i true" with given TARGET_NIC and NET_IRQ_CPULIST
DEFAULT_TARGET_NIC1=$TARGET_NIC_1 # TARGET NIC 1
DEFAULT_TARGET_NIC2=$TARGET_NIC_2 # TARGET NIC 2
DEFAULT_NET_IRQ_CPULIST1=67-71
DEFAULT_NET_IRQ_CPULIST2=86-95
DEFAULT_CLEAN_BRINGUP=1
DEFAULT_TRANSPORT=TCP
DEFAULT_TARGET_IP1=$TARGET_IP_1 # TARGET IP 1
DEFAULT_TARGET_IP2=$TARGET_IP_2 # TARGET IP 2
DEFAULT_SUBSYSTEM_COUNT=$SUBSYSTEM_CNT
DEFAULT_WRITE_BUFFER_SIZE_IN_MB=8192
DEFAULT_VOLUME_COUNT=$VOLUME_CNT
DEFAULT_VOLUME_SIZE=$VOLUME_BYTE_SIZE
DEFAULT_IRQ_DEDICATION=TRUE
DEFAULT_USER_DEVICE_LIST="-d unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3,unvme-ns-4,unvme-ns-5,unvme-ns-6,unvme-ns-7,unvme-ns-8,unvme-ns-9,unvme-ns-10,unvme-ns-11,unvme-ns-12,unvme-ns-13,unvme-ns-14,unvme-ns-15,unvme-ns-16,unvme-ns-17,unvme-ns-18,unvme-ns-19,unvme-ns-20,unvme-ns-21,unvme-ns-22,unvme-ns-23,unvme-ns-24,unvme-ns-25,unvme-ns-26,unvme-ns-27,unvme-ns-28,unvme-ns-29,unvme-ns-30"
DEFAULT_SPARE_DEVICE_LIST="-s unvme-ns-31"
ARRAYNAME=POSArray


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

ibofos_bringup(){

    if [ "$TRANSPORT" == "TCP" ] || [ "$TRANSPORT" == "tcp" ]; then 
        sudo $ROOT_DIR/test/system/network/tcp_tune.sh max
    	if [ "$IRQ_DEDICATION" == "TRUE" ] || [ "$IRQ_DEDICATION" == "true" ];then
            sudo systemctl stop irqbalance.service
	    echo "$ROOT_DIR/test/script/set_irq_affinity_cpulist.sh ${DEFAULT_NET_IRQ_CPULIST1} ${TARGET_NIC1}"
 
            sudo $ROOT_DIR/test/script/set_irq_affinity_cpulist.sh ${DEFAULT_NET_IRQ_CPULIST1} ${TARGET_NIC1}
            sudo $ROOT_DIR/test/script/set_irq_affinity_cpulist.sh ${DEFAULT_NET_IRQ_CPULIST2} ${TARGET_NIC2}
    	fi
        sudo $SPDK_DIR/scripts/rpc.py nvmf_create_transport -t $TRANSPORT -b 64 -n 4096
    else
        sudo $SPDK_DIR/scripts/rpc.py nvmf_create_transport -t $TRANSPORT -u 131072
    fi
    sudo $SPDK_DIR/scripts/rpc.py bdev_malloc_create -b uram0 $WRITE_BUFFER_SIZE_IN_MB 512
    sudo $ROOT_DIR/bin/cli device scan

    turn=0
    for i in `seq 1 $SUBSYSTEM_COUNT`
    do
        sudo $SPDK_DIR/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem$i -m 256 -a -s POS0000000000000$i -d POS_VOLUME_EXTENSION
        port=`expr $i % $PORT_COUNT + 1158`
        if [ $turn -eq 0 ]; then
            sudo $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem$i -t $TRANSPORT -a $TARGET_IP1 -s $port
            turn=1
        else
            sudo $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem$i -t $TRANSPORT -a $TARGET_IP2 -s $port
            turn=0
        fi
    done

    if [ "$CLEAN_BRINGUP" -eq 1 ]; then
        echo "poseidonos clean bringup"
	sudo $ROOT_DIR/bin/cli array reset
        sudo $ROOT_DIR/bin/cli array create -b uram0 $USER_DEVICE_LIST $SPARE_DEVICE_LIST --name $ARRAYNAME --raidtype RAID5
        sudo $ROOT_DIR/bin/cli array mount --name $ARRAYNAME

        for i in `seq 1 $VOLUME_COUNT`
        do
            ret=1
            while [ $ret -ne 0 ];
            do
                ret=$(sudo $ROOT_DIR/bin/cli --json volume create --name vol$i --size $VOLUME_SIZE --maxiops 0 --maxbw 0 --array $ARRAYNAME | jq ".Response.result.status.code")
                echo $ret
            done
            ret=1
            while [ $ret -ne 0 ];
            do
                ret=$(sudo $ROOT_DIR/bin/cli --json volume mount --name vol$i --array $ARRAYNAME | jq ".Response.result.status.code")
                echo $ret
            done
        done
    else
        echo "ibofos dirty bringup"
        #TODO : need to backup uram before load_array
        sudo $ROOT_DIR/bin/cli array mount --name $ARRAYNAME

        for i in `seq 1 $VOLUME_COUNT`
        do
            sudo $ROOT_DIR/bin/cli volume mount --name vol$i --array $ARRAYNAME
        done
    fi
    sudo $SPDK_DIR/scripts/rpc.py nvmf_get_subsystems
    sudo $ROOT_DIR/bin/cli logger set_level --level info

}

print_help()
{
    cat <<- END_OF_HELP

    usage: $(basename $0) [-h] [-c 0|1] [-t rdma|tcp] [-a xxx.xxx.xxx.xxx]
                          [-s N] [-w N] [-v N] [-i true|false]
                          [-u "unvme-ns-N,unvme-ns-N..."]
                          [-p "none" | "unvme-ns-N..."]
                          [-n "enp59s0"] [-q 46-55]

    Configures POS to be IO-Service-Ready.

    Options:
    -h: Print this help page
    -c: Clean bringup, Default: $DEFAULT_CLEAN_BRINGUP
    -t: Transport protocol, Default: $DEFAULT_TRANSPORT
    -a: IP Address1 of target server, Default: $DEFAULT_TARGET_IP1
    -b: IP Address2 of target server, Default: $DEFAULT_TARGET_IP2
    -s: # of Subsystem(s), Default: $DEFAULT_SUBSYSTEM_COUNT
    -w: Size for Write Buffer in MB, Default: $DEFAULT_WRITE_BUFFER_SIZE_IN_MB
    -v: # of Volume(s), Default: $DEFAULT_VOLUME_COUNT
    -i: Set Network IO IRQ dedication enabled for TCP, Default: $DEFAULT_IRQ_DEDICATION
    -u: Userdata Device list, Default: ${DEFAULT_USER_DEVICE_LIST:3}
    -p: Spare Device list, Default: ${DEFAULT_SPARE_DEVICE_LIST:3}
    -vs: Byte size of Volume(s), Default: $DEFAULT_VOLUME_SIZE
    -n: Target NIC1 Name, Defalut: $TARGET_NIC1
    -m: Target NIC2 Name, Defalut: $TARGET_NIC2
    -q: IRQ Cpu List, Default: $NET_IRQ_CPULIST

END_OF_HELP
}

while getopts c:t:a:s:w:v:S:i:u:p:q:n:h: ARG ; do
    case $ARG in
        c )
            CLEAN_BRINGUP=$OPTARG
            ;;
        t )
            TRANSPORT=$OPTARG
            ;;
        a )
            TARGET_IP1=$OPTARG
            ;;
        b )
            TARGET_IP2=$OPTARG
            ;;
        s )
            SUBSYSTEM_COUNT=$OPTARG
            ;;
        w )
            WRITE_BUFFER_SIZE_IN_MB=$OPTARG
            ;;
        v )
            VOLUME_COUNT=$OPTARG
            ;;
        S )
            VOLUME_SIZE=$OPTARG
            ;;
        i )
            IRQ_DEDICATION=$OPTARG
            ;;
        u )
            USER_DEVICE_LIST=$OPTARG
            ;;
        p )
            SPARE_DEVICE_LIST=$OPTARG
            ;;
        q )
            NET_IRQ_CPULIST=$OPTARG
            ;;
        n ) 
            TARGET_NIC1=$OPTARG
            ;;
        m ) 
            TARGET_NIC2=$OPTARG
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

log_normal "Checking variables..."
if [ -z $CLEAN_BRINGUP ]; then
CLEAN_BRINGUP=$DEFAULT_CLEAN_BRINGUP
log_error "1. CLEAN_BRINGUP empty, use default:"
fi
log_normal "1. CLEAN_BRINGUP => "$CLEAN_BRINGUP
echo ""

if [ -z $TRANSPORT ]; then
TRANSPORT=$DEFAULT_TRANSPORT
log_error "2. TRANSPORT empty, use default:"
fi
log_normal "2. TRANSPORT => "$TRANSPORT
echo ""

if [ -z $TARGET_IP1 ]; then
TARGET_IP1=$DEFAULT_TARGET_IP1
log_error "3.1 TARGET_IP1 empty, use default:"
fi
log_normal "3.1 TARGET_IP1 => "$TARGET_IP1
echo ""

if [ -z $TARGET_IP2 ]; then
TARGET_IP2=$DEFAULT_TARGET_IP2
log_error "3.2 TARGET_IP2 empty, use default:"
fi
log_normal "3.2 TARGET_IP2 => "$TARGET_IP2
echo ""

if [ -z $SUBSYSTEM_COUNT ]; then
SUBSYSTEM_COUNT=$DEFAULT_SUBSYSTEM_COUNT
log_error "4. SUBSYSTEM_COUNT empty, use default:"
fi
log_normal "4. SUBSYSTEM_COUNT => "$SUBSYSTEM_COUNT
echo ""

if [ -z $WRITE_BUFFER_SIZE_IN_MB ]; then
WRITE_BUFFER_SIZE_IN_MB=$DEFAULT_WRITE_BUFFER_SIZE_IN_MB
log_error "5. WRITE_BUFFER_SIZE_IN_MB empty, use default:"
fi
log_normal "5. WRITE_BUFFER_SIZE_IN_MB => "$WRITE_BUFFER_SIZE_IN_MB
echo ""

if [ -z $VOLUME_COUNT ]; then
VOLUME_COUNT=$DEFAULT_VOLUME_COUNT
log_error "6. VOLUME_COUNT empty, use default:"
fi
log_normal "6. VOLUME_COUNT => "$VOLUME_COUNT
echo ""

if [ -z $VOLUME_SIZE ]; then
VOLUME_SIZE=$DEFAULT_VOLUME_SIZE
log_error "7. VOLUME_SIZE empty, use default:"
fi
log_normal "7. VOLUME_SIZE => "$VOLUME_SIZE
echo ""

if [ -z $IRQ_DEDICATION ]; then
IRQ_DEDICATION=$DEFAULT_IRQ_DEDICATION
log_error "8. IRQ_DEDICATION empty. use default:"
fi
log_normal "8. IRQ_DEDICATION => "$IRQ_DEDICATION
echo ""

if [ -z $USER_DEVICE_LIST ]; then
USER_DEVICE_LIST=$DEFAULT_USER_DEVICE_LIST
log_error "9. USER_DEVICE_LIST empty. use default:"
else
USER_DEVICE_LIST="-d "$USER_DEVICE_LIST
fi
log_normal "9. USER_DEVICE_LIST => "${USER_DEVICE_LIST:3}
echo ""

if [ -z $SPARE_DEVICE_LIST ]; then
SPARE_DEVICE_LIST=$DEFAULT_SPARE_DEVICE_LIST
log_error "10. SPARE_DEVICE_LIST empty. use default:"
elif [ "$SPARE_DEVICE_LIST" == "NONE" ] || [ "$SPARE_DEVICE_LIST" == "none" ]; then
SPARE_DEVICE_LIST=""
else
SPARE_DEVICE_LIST="-s "$SPARE_DEVICE_LIST
fi
log_normal "10. SPARE_DEVICE_LIST => "${SPARE_DEVICE_LIST:3}
echo ""

if [ -z $TARGET_NIC1 ];then
TARGET_NIC1=$DEFAULT_TARGET_NIC1
log_error "11. TARGET_NIC1 empty. use default:"
fi
log_normal "11. TARGET_NIC1 => "$TARGET_NIC1
echo ""

if [ -z $TARGET_NIC2 ];then
TARGET_NIC2=$DEFAULT_TARGET_NIC2
log_error "11. TARGET_NIC2 empty. use default:"
fi
log_normal "11. TARGET_NIC2 => "$TARGET_NIC2
echo ""

if [ -z $NET_IRQ_CPULIST ];then
NET_IRQ_CPULIST=$DEFAULT_NET_IRQ_CPULIST
log_error "12. DEFAULT_NET_IRQ empty. use default:"
fi
log_normal "12. NET_IRQ_CPULIST => "$NET_IRQ_CPULIST
echo ""

ibofos_bringup

#!/bin/bash
# Note : increase VOLUME_COUNT & SUBSYSTEM_COUNT will make multiple volumes and namespace (1:1)

ROOT_DIR=$(readlink -f $(dirname $0))/../../../
SPDK_DIR=$ROOT_DIR/lib/spdk

PORT_COUNT=1
# Note: In case of tcp transport, network io irq can be manually controlled for better performance by issueing an option, "-i true" with given TARGET_NIC and NET_IRQ_CPULIST 
DEFAULT_TARGET_NIC=enp59s0
DEFAULT_NET_IRQ_CPULIST=46-55
DEFAULT_CLEAN_BRINGUP=1
DEFAULT_TRANSPORT=TCP
DEFAULT_TARGET_IP=127.0.0.1  # CI Server VM IP
DEFAULT_SUBSYSTEM_COUNT=1
DEFAULT_WRITE_BUFFER_SIZE_IN_MB=1024
DEFAULT_NUM_SHARED_BUFFER=4096
DEFAULT_VOLUME_COUNT=1
DEFAULT_VOLUME_SIZE=2147483648B
DEFAULT_IRQ_DEDICATION=FALSE
DEFAULT_USER_DEVICE_LIST="-d unvme-ns-0,unvme-ns-1,unvme-ns-2"
DEFAULT_SPARE_DEVICE_LIST="-s unvme-ns-3"
PMEM_ENABLED=0
ARRAYNAME=POSArray
CLI=${ROOT_DIR}/bin/poseidonos-cli
URAM_BLOCK_SIZE=512
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
            sudo $ROOT_DIR/test/script/set_irq_affinity_cpulist.sh ${NET_IRQ_CPULIST} ${TARGET_NIC}
        fi
        sudo ${CLI} subsystem create-transport -t ${TRANSPORT} -c 64 --num-shared-buf ${NUM_SHARED_BUFFER}
    else
        sudo $SPDK_DIR/scripts/rpc.py nvmf_create_transport -t $TRANSPORT -u 131072
    fi

    if [ ${PMEM_ENABLED} -eq 1 ]; then
        PMEM_POOL=/mnt/pmem0/pmem_pool
        if [ ! -e $PMEM_POOL ]; then
            sudo $SPDK_DIR/scripts/rpc.py bdev_pmem_create_pool ${PMEM_POOL} $WRITE_BUFFER_SIZE_IN_MB ${URAM_BLOCK_SIZE}
        fi
        sudo $SPDK_DIR/scripts/rpc.py bdev_pmem_create ${PMEM_POOL} -n pmem0
    else
        sudo ${CLI} device create -d uram0 --num-blocks $((WRITE_BUFFER_SIZE_IN_MB*1024*1024/URAM_BLOCK_SIZE)) --block-size ${URAM_BLOCK_SIZE} --device-type uram
    fi
	
    sudo $ROOT_DIR/bin/poseidonos-cli device scan

    for i in `seq 1 $SUBSYSTEM_COUNT`
    do
        sudo ${CLI} subsystem create -q nqn.2019-04.pos:subsystem$i -m 256 --allow-any-host --serial-number POS0000000000000$i --model-number POS_VOLUME_EXTENTION
        port=`expr $i % $PORT_COUNT + 1158`
        sudo ${CLI} subsystem add-listener -q nqn.2019-04.pos:subsystem$i -t $TRANSPORT -i $TARGET_IP -p $port 
    done

    if [ "$CLEAN_BRINGUP" -eq 1 ]; then
        echo "poseidonos clean bringup"
        sudo $ROOT_DIR/bin/poseidonos-cli devel resetmbr
        if [ ${PMEM_ENABLED} -eq 1 ]; then
            sudo $ROOT_DIR/bin/poseidonos-cli array create -b pmem0 $USER_DEVICE_LIST $SPARE_DEVICE_LIST --array-name $ARRAYNAME --raid RAID5
        else
            sudo $ROOT_DIR/bin/poseidonos-cli array create -b uram0 $USER_DEVICE_LIST $SPARE_DEVICE_LIST --array-name $ARRAYNAME --raid RAID5
        fi
        sudo $ROOT_DIR/bin/poseidonos-cli array mount --array-name $ARRAYNAME

        for i in `seq 1 $VOLUME_COUNT`
        do

            sudo $ROOT_DIR/bin/poseidonos-cli volume create --volume-name vol$i --size $VOLUME_SIZE --maxiops 0 --maxbw 0 --array-name  $ARRAYNAME
            sudo $ROOT_DIR/bin/poseidonos-cli volume mount --volume-name vol$i --array-name  $ARRAYNAME
        done
    else
        echo "poseidonos dirty bringup"
        #TODO : need to backup uram before load_array
        sudo $ROOT_DIR/bin/poseidonos-cli array mount --array-name $ARRAYNAME
        for i in `seq 1 $VOLUME_COUNT`
        do
            sudo $ROOT_DIR/bin/poseidonos-cli volume mount --volume-name vol$i --array-name  $ARRAYNAME
        done
    fi

    sudo ${CLI} subsystem list
    sudo $ROOT_DIR/bin/poseidonos-cli logger set-level --level debug

}

print_help()
{
    cat <<- END_OF_HELP

    usage: $(basename $0) [-h] [-c 0|1] [-t rdma|tcp] [-a xxx.xxx.xxx.xxx]
                          [-s N] [-w N] [-v N] [-i true|false]
                          [-u "unvme-ns-N,unvme-ns-N..."]
                          [-p "none" | "unvme-ns-N..."]
                          [-n "enp59s0"] [-q 46-55] [-b 4096]

    Configures POS to be IO-Service-Ready.

    Options:
    -h: Print this help page
    -c: Clean bringup, Default: $DEFAULT_CLEAN_BRINGUP
    -t: Transport protocol, Default: $DEFAULT_TRANSPORT
    -a: IP Address of target server, Default: $DEFAULT_TARGET_IP
    -s: # of Subsystem(s), Default: $DEFAULT_SUBSYSTEM_COUNT
    -w: Size for Write Buffer in MB, Default: $DEFAULT_WRITE_BUFFER_SIZE_IN_MB
    -v: # of Volume(s), Default: $DEFAULT_VOLUME_COUNT
    -i: Set Network IO IRQ dedication enabled for TCP, Default: $DEFAULT_IRQ_DEDICATION
    -u: Userdata Device list, Default: ${DEFAULT_USER_DEVICE_LIST:3}
    -p: Spare Device list, Default: ${DEFAULT_SPARE_DEVICE_LIST:3}
    -vs: Byte size of Volume(s), Default: $DEFAULT_VOLUME_SIZE
    -n: Target NIC Name, Defalut: $TARGET_NIC
    -q: IRQ Cpu List, Default: $NET_IRQ_CPULIST
    -b: Num Shared Buffer for Transport, Default: $NUM_SHARED_BUFFER
    -m: Using pmem_bdev instead of malloc bdev

END_OF_HELP
}

while getopts c:t:a:s:w:v:S:i:u:p:q:n:b:m:h: ARG ; do
    case $ARG in
        c )
            CLEAN_BRINGUP=$OPTARG
            ;;
        t )
            TRANSPORT=$OPTARG
            ;;
        a )
            TARGET_IP=$OPTARG
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
            TARGET_NIC=$OPTARG
            ;;
        b )
            NUM_SHARED_BUFFER=$OPTARG
            ;;
        m )
            PMEM_ENABLED=1
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

if [ -z $TARGET_IP ]; then
TARGET_IP=$DEFAULT_TARGET_IP
log_error "3. TARGET_IP empty, use default:"
fi
log_normal "3. TARGET_IP => "$TARGET_IP
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

if [ -z $TARGET_NIC ];then
TARGET_NIC=$DEFAULT_TARGET_NIC
log_error "11. TARGET_NIC empty. use default:"
fi
log_normal "11. TARGET_NIC => "$TARGET_NIC
echo ""

if [ -z $NET_IRQ_CPULIST ];then
NET_IRQ_CPULIST=$DEFAULT_NET_IRQ_CPULIST
log_error "12. DEFAULT_NET_IRQ empty. use default:"
fi
log_normal "12. NET_IRQ_CPULIST => "$NET_IRQ_CPULIST
echo ""

if [ -z $NUM_SHARED_BUFFER ];then
NUM_SHARED_BUFFER=$DEFAULT_NUM_SHARED_BUFFER
log_error "12. DEFAULT_NUM_SHARED_BUFFER empty. use default:"
fi
log_normal "12. NUM_SHARED_BUFFER => "$NUM_SHARED_BUFFER
echo ""

ibofos_bringup

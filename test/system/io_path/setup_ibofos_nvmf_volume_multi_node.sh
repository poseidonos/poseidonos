#!/bin/bash
# Note : increase VOLUME_COUNT & SUBSYSTEM_COUNT will make multiple volumes and namespace (1:1)

ROOT_DIR=$(readlink -f $(dirname $0))/../../../
SPDK_DIR=$ROOT_DIR/lib/spdk

# Note: In case of tcp transport, network io irq can be manually controlled for better performance by issueing an option, "-i true" with given TARGET_NIC and NET_IRQ_CPULIST 
DEFAULT_TARGET_NIC1=ens5f0
DEFAULT_TARGET_NIC2=ens17f0
DEFAULT_NET_IRQ_CPULIST1=65-71
DEFAULT_NET_IRQ_CPULIST2=88-94
DEFAULT_CLEAN_BRINGUP=1
DEFAULT_TRANSPORT=TCP
DEFAULT_TARGET_IP1=10.100.2.16 # CI Server VM IP
DEFAULT_TARGET_IP2=10.100.3.16 # CI Server VM IP
DEFAULT_SUBSYSTEM_COUNT1=33
DEFAULT_SUBSYSTEM_COUNT2=33
DEFAULT_VOLUME_COUNT1=33
DEFAULT_VOLUME_COUNT2=33
DEFAULT_WRITE_BUFFER_SIZE_IN_MB=4096
DEFAULT_NUM_SHARED_BUFFER=8192
DEFAULT_VOLUME_SIZE=2147483648
DEFAULT_IRQ_DEDICATION=TRUE
DEFAULT_USER_DEVICE_LIST1="-d unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3,unvme-ns-4,unvme-ns-5,unvme-ns-6,unvme-ns-7,unvme-ns-8,unvme-ns-9,unvme-ns-10,unvme-ns-11,unvme-ns-12,unvme-ns-13,unvme-ns-14,unvme-ns-15"
DEFAULT_USER_DEVICE_LIST2="-d unvme-ns-16,unvme-ns-17,unvme-ns-18,unvme-ns-19,unvme-ns-20,unvme-ns-21,unvme-ns-22,unvme-ns-23,unvme-ns-24,unvme-ns-25,unvme-ns-26,unvme-ns-27,unvme-ns-28,unvme-ns-29,unvme-ns-30,unvme-ns-31"
DEFAULT_SPARE_DEVICE_LIST=""
PMEM_ENABLED=0
ARRAYNAME1=POSArray1
ARRAYNAME2=POSArray2
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
            sudo $ROOT_DIR/test/script/set_irq_affinity_cpulist.sh ${NET_IRQ_CPULIST1} ${TARGET_NIC1}
            sudo $ROOT_DIR/test/script/set_irq_affinity_cpulist.sh ${NET_IRQ_CPULIST2} ${TARGET_NIC2}
        fi
        sudo $SPDK_DIR/scripts/rpc.py nvmf_create_transport -t $TRANSPORT -b 64 -n ${NUM_SHARED_BUFFER}
    else
        sudo $SPDK_DIR/scripts/rpc.py nvmf_create_transport -t $TRANSPORT -u 131072
    fi

    if [ ${PMEM_ENABLED} -eq 1 ]; then
        PMEM_POOL=/mnt/pmem0/pmem_pool
        if [ ! -e $PMEM_POOL ]; then
            sudo $SPDK_DIR/scripts/rpc.py bdev_pmem_create_pool ${PMEM_POOL} $WRITE_BUFFER_SIZE_IN_MB 512
        fi
        sudo $SPDK_DIR/scripts/rpc.py bdev_pmem_create ${PMEM_POOL} -n pmem0
    else
        sudo $ROOT_DIR/bin/poseidonos-cli device create --device-name uram0 --num-blocks 8388608 --block-size 512 --device-type uram --numa 0
        sudo $ROOT_DIR/bin/poseidonos-cli device create --device-name uram1 --num-blocks 8388608 --block-size 512 --device-type uram --numa 1
    fi

    sudo $ROOT_DIR/bin/poseidonos-cli device scan
    let SUBSYSTEM_COUNT_ARRAY_1_START=$SUBSYSTEM_COUNT1+1
    let SUBSYSTEM_COUNT_ARRAY_1_END=$SUBSYSTEM_COUNT1+$SUBSYSTEM_COUNT2
    for i in `seq 1 $SUBSYSTEM_COUNT1`
    do
        sudo $SPDK_DIR/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem$i -m 256 -a -s POS0000000000000$i -d POS_VOLUME_EXTENTION
        port=1158
        sudo $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem$i -t $TRANSPORT -a $TARGET_IP1 -s $port
    done
    for i in `seq $SUBSYSTEM_COUNT_ARRAY_1_START $SUBSYSTEM_COUNT_ARRAY_1_END`
    do
        sudo $SPDK_DIR/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem$i -m 256 -a -s POS0000000000000$i -d POS_VOLUME_EXTENTION:
        port=1159
        sudo $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem$i -t $TRANSPORT -a $TARGET_IP2 -s $port
    done

    if [ "$CLEAN_BRINGUP" -eq 1 ]; then
        echo "poseidonos clean bringup"
        sudo $ROOT_DIR/bin/poseidonos-cli devel resetmbr
        sudo $ROOT_DIR/bin/poseidonos-cli array create -b uram0 $USER_DEVICE_LIST1 --array-name $ARRAYNAME1 --raid RAID5
        sudo $ROOT_DIR/bin/poseidonos-cli array create -b uram1 $USER_DEVICE_LIST2 --array-name $ARRAYNAME2 --raid RAID5
        sudo $ROOT_DIR/bin/poseidonos-cli array mount --array-name $ARRAYNAME1
        sudo $ROOT_DIR/bin/poseidonos-cli array mount --array-name $ARRAYNAME2

        for i in `seq 1 $SUBSYSTEM_COUNT1`
        do
            sudo $ROOT_DIR/bin/poseidonos-cli volume create --volume-name vol$i --size $VOLUME_SIZE --maxiops 0 --maxbw 0 --array-name $ARRAYNAME1
            sudo $ROOT_DIR/bin/poseidonos-cli volume mount --volume-name vol$i --array-name $ARRAYNAME1
        done
   
        for i in `seq $SUBSYSTEM_COUNT_ARRAY_1_START $SUBSYSTEM_COUNT_ARRAY_1_END`
        do
            sudo $ROOT_DIR/bin/poseidonos-cli volume create --volume-name vol$i --size $VOLUME_SIZE --maxiops 0 --maxbw 0 --array-name $ARRAYNAME2
            sudo $ROOT_DIR/bin/poseidonos-cli volume mount --volume-name vol$i --array-name $ARRAYNAME2
        done
    else
        echo "poseidonos dirty bringup"
        sudo $ROOT_DIR/bin/poseidonos-cli array mount --array-name $ARRAYNAME
        for i in `seq 1 $VOLUME_COUNT`
        do
            sudo $ROOT_DIR/bin/poseidonos-cli volume mount --volume-name vol$i --array-name $ARRAYNAME
        done
    fi
    sudo $SPDK_DIR/scripts/rpc.py nvmf_get_subsystems
    sudo $ROOT_DIR/bin/poseidonos-cli logger set-level --level warning

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
    -a: IP Address of target server, Capital is node 2 Default: $DEFAULT_TARGET_IP
    -s: # of Subsystem(s), Capital is node 2 Default: $DEFAULT_SUBSYSTEM_COUNT
    -w: Size for Write Buffer in MB, Default: $DEFAULT_WRITE_BUFFER_SIZE_IN_MB
    -v: # of Volume(s), Capital is for node 2 Default: $DEFAULT_VOLUME_COUNT
    -i: Set Network IO IRQ dedication enabled for TCP, Default: $DEFAULT_IRQ_DEDICATION
    -u: Userdata Device list, Default: ${DEFAULT_USER_DEVICE_LIST:3}
    -p: Spare Device list, Default: ${DEFAULT_SPARE_DEVICE_LIST:3}
    -B: Byte size of Volume(s), Default: $DEFAULT_VOLUME_SIZE
    -n: Target NIC Name, Defalut: $TARGET_NIC
    -q: IRQ Cpu List, Default: $NET_IRQ_CPULIST
    -b: Num Shared Buffer for Transport, Default: $NUM_SHARED_BUFFER
    -m: Using pmem_bdev instead of malloc bdev

END_OF_HELP
}

while getopts c:t:a:A:s:S:w:v:V:B:i:u:U:p:q:n:b:m:h: ARG ; do
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
        A )
            TARGET_IP2=$OPTARG
            ;;
        s )
            SUBSYSTEM_COUNT1=$OPTARG
            ;;
        S )
            SUBSYSTEM_COUNT2=$OPTARG
            ;;
        w )
            WRITE_BUFFER_SIZE_IN_MB=$OPTARG
            ;;
        v )
            VOLUME_COUNT1=$OPTARG
            ;;
        V )
            VOLUME_COUNT2=$OPTARG
            ;;
        B )
            VOLUME_SIZE=$OPTARG
            ;;
        i )
			IRQ_DEDICATION=$OPTARG
			;;
		u )
			USER_DEVICE_LIST1=$OPTARG
			;;
		U )
			USER_DEVICE_LIST2=$OPTARG
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

if [ -z $TARGET_IP1 ]; then
TARGET_IP1=$DEFAULT_TARGET_IP1
log_error "3. TARGET_IP1 empty, use default:"
fi
log_normal "3. TARGET_IP1 => "$TARGET_IP1
echo ""

if [ -z $TARGET_IP2 ]; then
TARGET_IP2=$DEFAULT_TARGET_IP2
log_error "4. TARGET_IP2 empty, use default:"
fi
log_normal "4. TARGET_IP2 => "$TARGET_IP2
echo ""


if [ -z $SUBSYSTEM_COUNT1 ]; then
SUBSYSTEM_COUNT1=$DEFAULT_SUBSYSTEM_COUNT1
log_error "5. SUBSYSTEM_COUNT1 empty, use default:"
fi
log_normal "5. SUBSYSTEM_COUNT1 => "$SUBSYSTEM_COUNT1
echo ""

if [ -z $SUBSYSTEM_COUNT2 ]; then
SUBSYSTEM_COUNT2=$DEFAULT_SUBSYSTEM_COUNT2
log_error "6. SUBSYSTEM_COUNT2 empty, use default:"
fi
log_normal "6. SUBSYSTEM_COUNT2 => "$SUBSYSTEM_COUNT2
echo ""

if [ -z $WRITE_BUFFER_SIZE_IN_MB ]; then
WRITE_BUFFER_SIZE_IN_MB=$DEFAULT_WRITE_BUFFER_SIZE_IN_MB
log_error "7. WRITE_BUFFER_SIZE_IN_MB empty, use default:"
fi
log_normal "7. WRITE_BUFFER_SIZE_IN_MB => "$WRITE_BUFFER_SIZE_IN_MB
echo ""

if [ -z $VOLUME_COUNT1 ]; then
VOLUME_COUNT=$DEFAULT_VOLUME_COUNT1
log_error "8. VOLUME_COUNT1 empty, use default:"
fi
log_normal "8. VOLUME_COUNT1 => "$VOLUME_COUNT1
echo ""

if [ -z $VOLUME_COUNT2 ]; then
VOLUME_COUNT=$DEFAULT_VOLUME_COUNT2
log_error "9. VOLUME_COUNT2 empty, use default:"
fi
log_normal "9. VOLUME_COUNT2 => "$VOLUME_COUNT2
echo ""

if [ -z $VOLUME_SIZE ]; then
VOLUME_SIZE=$DEFAULT_VOLUME_SIZE
log_error "10. VOLUME_SIZE empty, use default:"
fi
log_normal "10. VOLUME_SIZE => "$VOLUME_SIZE
echo ""

if [ -z $IRQ_DEDICATION ]; then
IRQ_DEDICATION=$DEFAULT_IRQ_DEDICATION
log_error "11. IRQ_DEDICATION empty. use default:"
fi
log_normal "11. IRQ_DEDICATION => "$IRQ_DEDICATION
echo ""

if [ -z $USER_DEVICE_LIST1 ]; then
USER_DEVICE_LIST1=$DEFAULT_USER_DEVICE_LIST1
log_error "12. USER_DEVICE_LIST empty. use default:"
else
USER_DEVICE_LIST1="-d "$USER_DEVICE_LIST1
fi
log_normal "12. USER_DEVICE_LIST => "${USER_DEVICE_LIST1:3}
echo ""

if [ -z $USER_DEVICE_LIST2 ]; then
USER_DEVICE_LIST2=$DEFAULT_USER_DEVICE_LIST2
log_error "13. USER_DEVICE_LIST2 empty. use default:"
else
USER_DEVICE_LIST2="-d "$USER_DEVICE_LIST2
fi
log_normal "13. USER_DEVICE_LIST2 => "${USER_DEVICE_LIST2:3}
echo ""

if [ -z $TARGET_NIC ];then
TARGET_NIC1=$DEFAULT_TARGET_NIC1
TARGET_NIC2=$DEFAULT_TARGET_NIC2
log_error "14. TARGET_NIC empty. use default:"
fi
log_normal "14. TARGET_NIC => "$TARGET_NIC
echo ""

if [ -z $NET_IRQ_CPULIST ];then
NET_IRQ_CPULIST1=$DEFAULT_NET_IRQ_CPULIST1
NET_IRQ_CPULIST2=$DEFAULT_NET_IRQ_CPULIST2
log_error "15. DEFAULT_NET_IRQ empty. use default:"
fi
log_normal "15. NET_IRQ_CPULIST => "$NET_IRQ_CPULIST
echo ""

if [ -z $NUM_SHARED_BUFFER ];then
NUM_SHARED_BUFFER=$DEFAULT_NUM_SHARED_BUFFER
log_error "16. DEFAULT_NUM_SHARED_BUFFER empty. use default:"
fi
log_normal "16. NUM_SHARED_BUFFER => "$NUM_SHARED_BUFFER
echo ""

ibofos_bringup

#!/bin/bash
### BEGIN INIT INFO
# Provides:       poseidonos_auto_bringup
# Required-Start:    $local_fs $remote_fs $network $syslog
# Required-Stop:     $local_fs $remote_fs $network $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: starts pos_auto_bringup
# Description:       starts pos_auto_bringup using start-stop
### END INIT INFO

# Auto Bringup Usage

## Install auto bringup
### Edit ROOT_DIR of 5_auto_bringup.sh and execute below commands
### $ sudo cp 5_auto_bringup.sh /etc/init.d/pos_auto_bringup
### $ sudo chmod 755 /etc/init.d/pos_auto_bringup
### $ sudo update-rc.d pos_auto_bringup defaults

## Check auto bringup installed
### $ ls -al /etc/rc?.d | grep pos_auto_bringup
### There must be serveral symbolic results

## Check auto bringup log
### sudo journalctl -u pos_auto_bringup.service

## Uninstall
### $ sudo update-rc.d -f pos_auto_bringup remove

## Check auto bringup uninstalled
### $ ls -al /etc/rc?.d | grep pos_auto_bringup
### There must be no results

ROOT_DIR=/home/psd/poseidonos/
SPDK_DIR=$ROOT_DIR/lib/spdk
logfile=pos.log
binary_name=poseidonos

PORT_COUNT=1
PORT=1158
# Note: In case of tcp transport, network io irq can be manually controlled for better performance by issueing an option, "-i true" with given TARGET_NIC and NET_IRQ_CPULIST
DEFAULT_NET_IRQ_CPULIST1=67-71
DEFAULT_NET_IRQ_CPULIST2=86-95
DEFAULT_TRANSPORT=TCP
DEFAULT_WRITE_BUFFER_SIZE_IN_MB=8192

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

setup_environment()
{
    ${ROOT_DIR}/script/setup_env.sh
    rm -rf /dev/shm/ibof_nvmf_trace*
}

execute_poseidonos()
{
    if [ -f ${ROOT_DIR}/bin/$binary_name ];
    then
        echo "Execute poseidonos"
        nohup ${ROOT_DIR}/bin/$binary_name &>> ${ROOT_DIR}/script/${logfile} &
    else
        echo "No executable poseidonos file"
        exit -1
    fi
}

check_started()
{
    result=`${ROOT_DIR}/bin/cli system info --json | jq '.Response.data.version' 2>/dev/null`

    if [ -z ${result} ] || [ ${result} == '""' ];
    then
        return 0
    else
        return 1
    fi
}

wait_started()
{
    check_started
    while [ $? -eq 0 ];
    do
        echo "Wait poseidonos"
        sleep 0.5
        check_started
    done
}

poseidonos_bringup(){
    echo "$SPDK_DIR/scripts/rpc.py bdev_malloc_create -b uram0 $WRITE_BUFFER_SIZE_IN_MB 512"
    sudo $SPDK_DIR/scripts/rpc.py bdev_malloc_create -b uram0 $WRITE_BUFFER_SIZE_IN_MB 512

    echo "$SPDK_DIR/scripts/rpc.py bdev_malloc_create -b uram1 $WRITE_BUFFER_SIZE_IN_MB 512"
    sudo $SPDK_DIR/scripts/rpc.py bdev_malloc_create -b uram1 $WRITE_BUFFER_SIZE_IN_MB 512

    if [ "$TRANSPORT" == "TCP" ] || [ "$TRANSPORT" == "tcp" ]; then 
        sudo $ROOT_DIR/test/system/network/tcp_tune.sh max
        if [ "$IRQ_DEDICATION" == "TRUE" ] || [ "$IRQ_DEDICATION" == "true" ];then
            sudo systemctl stop irqbalance.service
            echo "$ROOT_DIR/test/script/set_irq_affinity_cpulist.sh ${DEFAULT_NET_IRQ_CPULIST1} ${TARGET_NIC1}"
            echo "$ROOT_DIR/test/script/set_irq_affinity_cpulist.sh ${DEFAULT_NET_IRQ_CPULIST2} ${TARGET_NIC2}"
            sudo $ROOT_DIR/test/script/set_irq_affinity_cpulist.sh ${DEFAULT_NET_IRQ_CPULIST1} ${TARGET_NIC1}
            sudo $ROOT_DIR/test/script/set_irq_affinity_cpulist.sh ${DEFAULT_NET_IRQ_CPULIST2} ${TARGET_NIC2}
        fi
        echo "$SPDK_DIR/scripts/rpc.py nvmf_create_transport -t $TRANSPORT -b 64 -n 4096"
        sudo $SPDK_DIR/scripts/rpc.py nvmf_create_transport -t $TRANSPORT -b 64 -n 4096
    else
        echo "$SPDK_DIR/scripts/rpc.py nvmf_create_transport -t $TRANSPORT -u 131072"
        sudo $SPDK_DIR/scripts/rpc.py nvmf_create_transport -t $TRANSPORT -u 131072
    fi
    echo "$ROOT_DIR/bin/cli device scan"
    sudo $ROOT_DIR/bin/cli device scan

    echo "poseidonos auto bringup"
    echo "$ROOT_DIR/bin/cli array list"
    volCnt=0
    echo "sudo $ROOT_DIR/bin/cli array list --json | jq '.Response.result.data.arrayList | length'"
    arrayNum=`sudo $ROOT_DIR/bin/cli array list --json | jq '.Response.result.data.arrayList | length'`
    for i in `seq 0 $((arrayNum-1))`
    do
        echo "sudo $ROOT_DIR/bin/cli array list --json | jq '.Response.result.data.arrayList | .[$i].name'"
        ARRAYNAME=`sudo $ROOT_DIR/bin/cli array list --json | jq -r ".Response.result.data.arrayList | .[$i].name"`
        echo "$ROOT_DIR/bin/cli array mount --name $ARRAYNAME"
        sudo $ROOT_DIR/bin/cli array mount --name $ARRAYNAME

        echo "sudo $ROOT_DIR/bin/cli volume list --json | jq '.Response.result.data.volumes | length'"
        volumeNum=`sudo $ROOT_DIR/bin/cli volume list --array $ARRAYNAME --json | jq '.Response.result.data.volumes | length'`
        for j in `seq 0 $((volumeNum-1))`
        do
            echo "sudo $ROOT_DIR/bin/cli volume list --array $ARRAYNAME --json | jq '.Response.result.data.volumes | .[$j].name'"
            volumeName=`sudo $ROOT_DIR/bin/cli volume --array $ARRAYNAME list --json | jq -r ".Response.result.data.volumes | .[$j].name"`
            echo "$SPDK_DIR/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem$volCnt -m 256 -a -s POS0000000000000$volCnt -d POS_VOLUME_EXTENSION"
            sudo $SPDK_DIR/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem$volCnt -m 256 -a -s POS0000000000000$volCnt -d POS_VOLUME_EXTENSION
            echo "sudo $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem$volCnt -t $TRANSPORT -a $TARGET_IP1 -s $PORT"
            sudo $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem$volCnt -t $TRANSPORT -a $TARGET_IP1 -s $PORT
            echo "$ROOT_DIR/bin/cli volume mount --name $volumeName --array $ARRAYNAME"
            sudo $ROOT_DIR/bin/cli volume mount --name $volumeName --array $ARRAYNAME
            ((volCnt++))
        done
    done

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

while getopts t:w:q:h: ARG ; do
    case $ARG in
        t )
            TRANSPORT=$OPTARG
            ;;
        w )
            WRITE_BUFFER_SIZE_IN_MB=$OPTARG
            ;;
        q )
            NET_IRQ_CPULIST=$OPTARG
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

if [ -z $TRANSPORT ]; then
TRANSPORT=$DEFAULT_TRANSPORT
log_error "1. TRANSPORT empty, use default:"
fi
log_normal "1. TRANSPORT => "$TRANSPORT
echo ""

if [ -z $WRITE_BUFFER_SIZE_IN_MB ]; then
WRITE_BUFFER_SIZE_IN_MB=$DEFAULT_WRITE_BUFFER_SIZE_IN_MB
log_error "2. WRITE_BUFFER_SIZE_IN_MB empty, use default:"
fi
log_normal "2. WRITE_BUFFER_SIZE_IN_MB => "$WRITE_BUFFER_SIZE_IN_MB
echo ""

setup_environment
execute_poseidonos
wait_started
echo "poseidonos is running in background...logfile=${logfile}"
poseidonos_bringup

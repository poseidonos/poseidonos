#!/bin/bash
#
# nvmf_initiator_nvme_cli.sh
#

ROOT_DIR=$(readlink -f $(dirname $0))/../../../
SPDK_DIR=$ROOT_DIR/lib/spdk

NVMF_TARGET_NQN_PREFIX=nqn.2019-04.pos:subsystem
NVMF_TARGET_PORT=1158
NVMF_INITIATOR_APP=nvme

DEFAULT_VOLUME_COUNT=1
DEFAULT_TEST_COUNT=1
DEFAULT_NVMF_TRANSPORT=tcp
DEFAULT_SERVER_IP=10.100.11.1   # CI Server VM IP

###################### ^^^ USER CONFIGURABLES ^^^ ######################
########################################################################

RED_COLOR="\033[1;31m"
GREEN_COLOR="\033[0;32m"
RESET_COLOR="\033[0;0m"

log_normal(){
    echo -e $GREEN_COLOR$1$RESET_COLOR
}

log_error(){
    echo -e $RED_COLOR$1$RESET_COLOR
}

discovery(){
    log_normal "Discovery Controller..."
    echo $NVMF_INITIATOR_APP discover -t $NVMF_TRANSPORT -a $SERVER_IP -s $NVMF_TARGET_PORT
    $NVMF_INITIATOR_APP discover -t $NVMF_TRANSPORT -a $SERVER_IP -s $NVMF_TARGET_PORT
    if [ $? = 0 ]; then
        log_normal "Discovery Succeeded"
    else
        log_error "Discovery failed"
        exit 1
    fi
    echo ""
}

connect(){
    log_normal "Connecting to NVMeoF subsystem..."
    for i in `seq 1 $VOLUME_COUNT`
    do
        echo $NVMF_INITIATOR_APP connect -t $NVMF_TRANSPORT -n "$NVMF_TARGET_NQN_PREFIX$i" -a $SERVER_IP -s $NVMF_TARGET_PORT
        $NVMF_INITIATOR_APP connect -t $NVMF_TRANSPORT -n "$NVMF_TARGET_NQN_PREFIX$i" -a $SERVER_IP -s $NVMF_TARGET_PORT
        if [ $? = 0 ]; then
            log_normal "Connection succeeded - Volume #$i"
        else
            log_error "Connection failed - Volume #$i"
            exit 1
        fi
        echo ""
    done
}

disconnect(){
    log_normal "Disconnecting from NVMeoF subsystem..."
    for i in `seq 1 $VOLUME_COUNT`
    do
        echo $NVMF_INITIATOR_APP disconnect -n "$NVMF_TARGET_NQN_PREFIX$i"
        $NVMF_INITIATOR_APP disconnect -n "$NVMF_TARGET_NQN_PREFIX$i"
        if [ $? = 0 ]; then
            log_normal "Disconnection succeeded - Volume #$i"
        else
            log_error "Disconnection failed - Volume #$i"
            exit 1
        fi
        echo ""
    done
}

connect_once(){
    discovery
    disconnect
    connect
    disconnect
}

connect_multiple(){
    SET=$(seq 1 $TEST_COUNT)
    for i in $SET
    do
    log_normal "### Test"$i" ###"
        connect_once
        sleep 0.5
    done
}

print_help()
{
    cat <<- END_OF_HELP

    usage: $(basename $0) [-h] [-v N] [-r N] [-t rdma|tcp] [-a xxx.xxx.xxx.xxx]

    Tests NVMeoF discovery, connection and disconnection.

    Options:
    -h: Print this help page
    -v: # of Volume(s) to test, Default: $DEFAULT_VOLUME_COUNT
    -r: # of replay the test, Default: $DEFAULT_TEST_COUNT
    -t: Transport protocol, Default: $DEFAULT_NVMF_TRANSPORT
    -a: IP Address of target server, Default: $DEFAULT_SERVER_IP

END_OF_HELP
}

while getopts v:r:t:a:h ARG ; do
    case $ARG in
        v )
            VOLUME_COUNT=$OPTARG
            ;;
        r )
            TEST_COUNT=$OPTARG
            ;;
        t )
            NVMF_TRANSPORT=$OPTARG
            ;;
        a )
            SERVER_IP=$OPTARG
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
if [ -z $VOLUME_COUNT ]; then
VOLUME_COUNT=$DEFAULT_VOLUME_COUNT
log_error "1. VOLUME_COUNT empty, use default:"
fi
log_normal "1. VOLUME_COUNT => "$VOLUME_COUNT
echo ""

if [ -z $TEST_COUNT ]; then
TEST_COUNT=$DEFAULT_TEST_COUNT
log_error "2. TEST_COUNT empty, use default:"
fi
log_normal "2. TEST_COUNT => "$TEST_COUNT
echo ""

if [ -z $NVMF_TRANSPORT ]; then
NVMF_TRANSPORT=$DEFAULT_NVMF_TRANSPORT
log_error "3. NVMF_TRANSPORT empty, use default:"
fi
NVMF_TRANSPORT=$(echo $NVMF_TRANSPORT | tr '[:upper:]' '[:lower:]')
log_normal "3. NVMF_TRANSPORT => "$NVMF_TRANSPORT
echo ""

if [ -z $SERVER_IP ]; then
SERVER_IP=$DEFAULT_SERVER_IP
log_error "4. SERVER_IP empty, use default: "
fi
log_normal "4. SERVER_IP => "$SERVER_IP
echo ""

log_normal "Testing NVMeoF Connection and Disconnection..."
connect_multiple

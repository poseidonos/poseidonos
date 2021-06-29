#!/bin/bash
# Note : increase VOLUME_COUNT & SUBSYSTEM_COUNT will make multiple volumes and namespace (1:1)

ROOT_DIR=$(readlink -f $(dirname $0))/../../
SPDK_DIR=$ROOT_DIR/lib/spdk-19.10

PORT_COUNT=1
# Note: In case of tcp transport, network io irq can be manually controlled for better performance by issueing an option, "-i true" with given TARGET_NIC and NET_IRQ_CPULIST
DEFAULT_TARGET_IP1=10.100.2.16 # TARGET IP 1
DEFAULT_TARGET_IP2=10.100.3.16 # TARGET IP 2
DEFAULT_VOLUME_COUNT=47
DEFAULT_SEQ_IO_TIME=60
DEFAULT_RAND_IO_TIME=60
DEFAULT_VOLUME_SIZE=1
DEFAULT_VDBENCH_SUB_INITIATOR_IP=10.1.2.31
DEFAULT_VDBENCH_DIR=/home/psd/vdbench/
DEFAULT_FIO_DIR=/home/psd/fio_conf/
DEFAULT_FIO_IO_ENGINE="/home/psd/ibofos/lib/spdk/examples/nvme/fio_plugin/fio_plugin"
DEFAULT_NVME_LIST_1="/dev/nvme0n1 /dev/nvme1n1"
DEFAULT_NVME_LIST_2="/dev/nvme0n1 /dev/nvme1n1"

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

make_fio_config()
{
sw_files=("sw_tcp_init1.conf" "sw_tcp_init2.conf")
rw_files=("rw_tcp_init1.conf" "rw_tcp_init2.conf")
general_configs=("[global]" "ioengine=${FIO_IO_ENGINE}" "size=100%" "thread=1" "serialize_overlap=0" "group_reporting=1" "direct=1" "numjobs=1" "ramp_time=0" "time_based=1" "log_avg_msec=2000")
sw_configs=("rwmixread=0" "readwrite=rw" "iodepth=4" "runtime=${SEQ_IO_TIME}" "io_size=${VOLUME_SIZE}g" "bs=128k" "write_bw_log=seqrw.log" "write_iops_log=seqrw.log")
rw_configs=("rwmixread=70" "readwrite=randrw" "iodepth=128" "runtime=${RAND_IO_TIME}" "io_size=${VOLUME_SIZE}g" "bs=4k" "write_bw_log=randrw.log" "write_iops_log=randrw.log")

    for file in ${sw_files[@]}
    do
        echo "" > ${file}
	for line in ${general_configs[@]}
	do
            echo ${line} >> ${file}
	done

	for line in ${sw_configs[@]}
        do
            echo ${line} >> ${file}
        done
    done

    for file in ${rw_files[@]}
    do
        echo "" > ${file}
        for line in ${general_configs[@]}
	do
            echo ${line} >> ${file}
	done

        for line in ${rw_configs[@]}
        do
            echo ${line} >> ${file}
        done
    done


    for i in `seq 1 $VOLUME_COUNT`
    do
        if [ `expr ${i} % 2` -eq 1 ]
        then
            echo [test${i}] >> ${sw_files[0]}
            echo [test${i}] >> ${rw_files[0]}
            echo filename=trtype=tcp adrfam=ipv4 traddr=${TARGET_IP1} trsvcid=1158 subnqn=nqn.2019-04.ibof\\:subsystem${i} ns=1 >> ${sw_files[0]}
            echo filename=trtype=tcp adrfam=ipv4 traddr=${TARGET_IP1} trsvcid=1158 subnqn=nqn.2019-04.ibof\\:subsystem${i} ns=1 >> ${rw_files[0]}
        else
            echo [test${i}] >> ${sw_files[1]}
            echo [test${i}] >> ${rw_files[1]}
            echo filename=trtype=tcp adrfam=ipv4 traddr=${TARGET_IP2} trsvcid=1158 subnqn=nqn.2019-04.ibof\\:subsystem${i} ns=1 >> ${sw_files[1]}
            echo filename=trtype=tcp adrfam=ipv4 traddr=${TARGET_IP2} trsvcid=1158 subnqn=nqn.2019-04.ibof\\:subsystem${i} ns=1 >> ${rw_files[1]}
        fi
    done
}

make_test_script()
{
    make_fio_config
}

print_help()
{
cat <<- END_OF_HELP

usage: $(basename $0) [-h] [-a xxx.xxx.xxx.xxx] [-b xxx.xxx.xxx.xxx] [-p xxx.xxx.xxx.xxx]
[-S N] [-v N]

Create io test config to be IO-TEST.

Options:
-h: Print this help page
-a: IP Address1 of target server, Default: $DEFAULT_TARGET_IP1
-b: IP Address2 of target server, Default: $DEFAULT_TARGET_IP2
-v: # of Volume(s), Default: $DEFAULT_VOLUME_COUNT
-S: Byte size of Volume(s), Default: $DEFAULT_VOLUME_SIZE
-s: seq io time(s), Default: $DEFAULT_SEQ_IO_TIME
-r: rand io time(s), Default: $DEFAULT_RAND_IO_TIME
-d: test vdbench directory, Default: $DEFAULT_VDBENCH_DIR
-f: test fio directory, Default: $DEFAULT_FIO_DIR
-e: fio io engine directory, Default: $DEFAULT_FIO_IO_ENGINE

END_OF_HELP
}

while getopts a:b:s:v:S:r:p:h:d:f:e:l:L: ARG ; do
case $ARG in
a )
TARGET_IP1=$OPTARG
;;
b )
TARGET_IP2=$OPTARG
;;
v )
VOLUME_COUNT=$OPTARG
;;
S )
VOLUME_SIZE=$OPTARG
;;
s )
SEQ_IO_TIME=$OPTARG
;;
r )
RAND_IO_TIME=$OPTARG
;;
p )
VDBENCH_SUB_INITIATOR_IP=$OPTARG
;;
d )
VDBENCH_DIR=$OPTARG
;;
f )
FIO_DIR=$OPTARG
;;
e )
FIO_IO_ENGINE=$OPTARG
;;
l )
NVME_LIST_1=$OPTARG
;;
L )
NVME_LIST_2=$OPTARG
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

if [ -z $TARGET_IP1 ]; then
TARGET_IP1=$DEFAULT_TARGET_IP1
log_error "1.1 TARGET_IP1 empty, use default:"
fi
log_normal "1.1 TARGET_IP1 => "$TARGET_IP1
echo ""

if [ -z $TARGET_IP2 ]; then
TARGET_IP2=$DEFAULT_TARGET_IP2
log_error "1.2 TARGET_IP2 empty, use default:"
fi
log_normal "1.2 TARGET_IP2 => "$TARGET_IP2
echo ""

if [ -z $VOLUME_COUNT ]; then
VOLUME_COUNT=$DEFAULT_VOLUME_COUNT
log_error "2. VOLUME_COUNT empty, use default:"
fi
log_normal "2. VOLUME_COUNT => "$VOLUME_COUNT
echo ""

if [ -z $VOLUME_SIZE ]; then
VOLUME_SIZE=$DEFAULT_VOLUME_SIZE
log_error "3. VOLUME_SIZE empty, use default:"
fi
log_normal "3. VOLUME_SIZE => "$VOLUME_SIZE
echo ""

if [ -z $SEQ_IO_TIME ]; then
SEQ_IO_TIME=$DEFAULT_SEQ_IO_TIME
log_error "4. SEQ_IO_TIME empty, use default:"
fi
log_normal "4. SEQ_IO_TIME => "$SEQ_IO_TIME
echo ""

if [ -z $RAND_IO_TIME ]; then
RAND_IO_TIME=$DEFAULT_RAND_IO_TIME
log_error "5. RAND_IO_TIME empty, use default:"
fi
log_normal "5. RAND_IO_TIME => "$RAND_IO_TIME
echo ""

if [ -z $VDBENCH_SUB_INITIATOR_IP ]; then
VDBENCH_SUB_INITIATOR_IP=$DEFAULT_VDBENCH_SUB_INITIATOR_IP
log_error "6. VDBENCH_SUB_INITIATOR_IP empty, use default:"
fi
log_normal "6. VDBENCH_SUB_INITIATOR_IP => "$VDBENCH_SUB_INITIATOR_IP
echo ""

if [ -z $VDBENCH_DIR ]; then
VDBENCH_DIR=$DEFAULT_VDBENCH_DIR
log_error "7. VDBENCH_DIR empty, use default:"
fi
log_normal "7. VDBENCH_DIR => "$VDBENCH_DIR
echo ""

if [ -z $FIO_DIR ]; then
FIO_DIR=$DEFAULT_FIO_DIR
log_error "8. FIO_DIR empty, use default:"
fi
log_normal "8. FIO_DIR => "$FIO_DIR
echo ""

if [ -z $FIO_IO_ENGINE ]; then
FIO_IO_ENGINE=$DEFAULT_FIO_IO_ENGINE
log_error "9. FIO_IO_ENGINE empty, use default:"
fi
log_normal "9. FIO_IO_ENGINE => "$FIO_IO_ENGINE
echo ""

if [ -z $NVME_LIST_1 ]; then
NVME_LIST_1=$DEFAULT_NVME_LIST_1
log_error "10. NVME_LIST_1 empty, use default:"
fi
for line in ${NVME_LIST_1[@]}
do
log_normal "10. NVME_LIST_1 => "$line
done
echo ""

if [ -z $NVME_LIST_2 ]; then
NVME_LIST_2=$DEFAULT_NVME_LIST_2
log_error "11. NVME_LIST_2 empty, use default:"
fi
for line in ${NVME_LIST_2[@]}
do
log_normal "11. NVME_LIST_2 => "$line
done
echo ""


make_test_script

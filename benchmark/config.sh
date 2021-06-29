#!/bin/bash

## How to run this code? ##
##                       ##
##   source ./config.sh  ##
##                       ##
###########################

export ROOT_DIR=$(readlink -f $(dirname $0))/../../

# Network Environment
export TARGET_IP_1=10.100.2.16
export TARGET_IP_2=10.100.3.16
export TARGET_NIC_1=ens5f0
export TARGET_NIC_2=ens17f0

export INIT_1_ID=psd
export INIT_2_ID=psd
export INIT_1_PW=psd
export INIT_2_PW=psd
export INIT_1_IP=10.1.2.30
export INIT_2_IP=10.1.2.31

# PoseidonOS Configuration
export SUBSYSTEM_CNT=43
export VOLUME_CNT=43
export VOLUME_GB_SIZE=1440
export VOLUME_BYTE_SIZE=$((VOLUME_GB_SIZE*1024*1024*1024))

# Common POC Variables
export SEQ_IO_TIME=0
export RAND_IO_TIME=7200

export INIT1_FILES="
sw_tcp_init1.conf
rw_tcp_init1.conf
"
export INIT2_FILES="
sw_tcp_init2.conf
rw_tcp_init2.conf
"

#Benchmark1 (Default I/O) Variables
export INIT1_FIO_SCRIPT_DIR=/home/psd/poseidonos
export INIT2_FIO_SCRIPT_DIR=/home/psd/poseidonos
export INIT1_FIO_SCRIPT_FILE=perf_200g.py
export INIT2_FIO_SCRIPT_FILE=perf_200g.py
export RAMP_TIME=5
export IO_RUN_TIME=20

#Benchmark3 Vdbench Variables
export DEMO_TEST=0

#Benchmark 4~5 Variables
export INIT1_VDBENCH_DIR=/home/psd/vdbench
export INIT2_VDBENCH_DIR=/home/psd/vdbench
export INIT1_FIO_CONF_DIR=/home/psd/fio_conf
export INIT2_FIO_CONF_DIR=/home/psd/fio_conf

export INIT1_FIO_ENGINE=/home/psd/poseidonos/lib/spdk/examples/nvme/fio_plugin/fio_plugin
export INIT2_FIO_ENGINE=/home/psd/poseidonos/lib/spdk/examples/nvme/fio_plugin/fio_plugin

#Benchmark4 (GC, 10hr) Variables
export VDBENCH_SUB_INIT_IP=${INIT_1_IP}

#second only even number
export LONGTERM_SEQ_IO_TIME=10800
export LONGTERM_RAND_IO_TIME=25200

#Benchmark5 (GC, large volume delete) Variables
export DELETE_VOLUME_CNT=30
export REMAIN_VOLUME_CNT=$((VOLUME_CNT-DELETE_VOLUME_CNT))

#second only even number
export WRITE_FILL_SEQ_IO_TIME=10800
export WRITE_FILL_RAND_IO_TIME=0
export VOL_DEL_SEQ_IO_TIME=0
export VOL_DEL_RAND_IO_TIME=43200

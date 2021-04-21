#!/bin/bash

ROOTDIR=$(readlink -f $(dirname $0))/..
DIR=/tmp
INFO_POSTFIX=uram.info
DATA_POSTFIX=uram.data

# |    PID    | Start page # | Page count |
#
# PID: latest pid for poseidonos
# Start page #: latest logical page number in hugepage for uram
# Page count: # of hugepages to backup for uram

for uram_name in `ls ${DIR}/*.${INFO_POSTFIX} | awk -F[./] '{print $3}'`
do
    URAM_INFO_FILE=${DIR}/${uram_name}.${INFO_POSTFIX}
    URAM_DATA_FILE=${DIR}/${uram_name}.${DATA_POSTFIX}
    read -a array < ${URAM_INFO_FILE}

    PID=${array[0]}
    URAM_PAGE_START_OFFSET=${array[2]}
    URAM_PAGE_COUNT=${array[3]}

    echo "Latest PID of poseidonos: $PID"
    echo "Latest Logical page number in hugepage for uram: $URAM_PAGE_START_OFFSET"
    echo "Page counts of uram data: $URAM_PAGE_COUNT"
    echo "Starting to backup URAM contents.."

    FIRST_BIN_FILE_NAME=/dev/hugepages/spdk_pid${PID}map_${URAM_PAGE_START_OFFSET}
    if [ -f $FIRST_BIN_FILE_NAME ]; then
        sudo dd if="/dev/hugepages/spdk_pid${PID}map_${URAM_PAGE_START_OFFSET}" of=${URAM_DATA_FILE} bs=2M count=1 status=none

        FILE_OFFSET=1
        for pageIndex in `seq $(($URAM_PAGE_START_OFFSET+1)) $(($URAM_PAGE_START_OFFSET+$URAM_PAGE_COUNT-1))`
        do
            sudo dd if="/dev/hugepages/spdk_pid${PID}map_${pageIndex}" of=${URAM_DATA_FILE} bs=2M count=1 seek=$FILE_OFFSET conv=nocreat,notrunc  status=none &
            FILE_OFFSET=$((FILE_OFFSET+1))
        done
        wait
        echo "URAM backup has been completed!"
    else
        echo "No data to backup"
    fi
done

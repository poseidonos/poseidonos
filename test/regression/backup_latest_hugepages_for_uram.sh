#!/bin/bash

URAM_META_FILE=/tmp/uram_hugepage
OUTPUT_DIR=/etc/uram_backup
OUTPUT_FILE_NAME=$OUTPUT_DIR/uram_backup.bin

if [ -d "${OUTPUT_DIR}" ]; then
    echo ${OUTPUT_DIR} exists.
else
    sudo mkdir ${OUTPUT_DIR}
fi

sudo umount ${OUTPUT_DIR}
echo umount done.

sudo mount -t tmpfs -o size=5G tmpfs ${OUTPUT_DIR}
echo mount done.

# |    PID    | Start page # | Page count |
#
# PID: latest pid for ibofos
# Start page #: latest logical page number in hugepage for uNVRAM
# Page count: # of hugepages to backup for uNVRAM

read -a array < $URAM_META_FILE

PID=${array[0]}
URAM_PAGE_START_OFFSET=${array[1]}
URAM_PAGE_COUNT=${array[2]}

echo "Latest PID of ibofos: $PID"
echo "Latest Logical page number in hugepage for uNVRAM: $URAM_PAGE_START_OFFSET"
echo "Page counts of uNVRAM data: $URAM_PAGE_COUNT"

# echo $URAM_PAGE_START_OFFSET
# echo $(($URAM_PAGE_START_OFFSET+$URAM_PAGE_COUNT-1))

echo "Starting to backup UNVRAM contents.."
#echo "/dev/hugepages/spdk_pid${PID}map_${URAM_PAGE_START_OFFSET}"
sudo dd if="/dev/hugepages/spdk_pid${PID}map_${URAM_PAGE_START_OFFSET}" of=$OUTPUT_FILE_NAME bs=2M count=1 status=none

for pageIndex in `seq $(($URAM_PAGE_START_OFFSET+1)) $(($URAM_PAGE_START_OFFSET+$URAM_PAGE_COUNT-1))`
do
    #echo "/dev/hugepages/spdk_pid${PID}map_${pageIndex}"
    sudo dd if="/dev/hugepages/spdk_pid${PID}map_${pageIndex}" of=$OUTPUT_FILE_NAME bs=2M count=1 oflag=append conv=nocreat,notrunc  status=none
done


echo "UNVRAM backup has been completed!"
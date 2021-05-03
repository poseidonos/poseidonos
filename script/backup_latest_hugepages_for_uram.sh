#!/bin/bash

ROOTDIR=$(readlink -f $(dirname $0))/..
DIR=/tmp
INFO_POSTFIX=uram.info
DATA_POSTFIX=uram.data

# | PID | Memory Address | Start page # | Page count |
#
# PID: latest pid for poseidonos
# Start page #: latest logical page number in hugepage for uram
# Page count: # of hugepages to backup for uram

RED_COLOR="\033[1;31m"
GREEN_COLOR="\033[0;32m"
RESET_COLOR="\033[0;0m"

print_error(){
	DEVICE_NAME=$1
	echo -e ${RED_COLOR}Failed to backup ${DEVICE_NAME} hugepages${RESET_COLOR}
}

for uram_name in `ls ${DIR}/*.${INFO_POSTFIX} | awk -F[./] '{print $3}'`
do
	URAM_INFO_FILE=${DIR}/${uram_name}.${INFO_POSTFIX}
	URAM_DATA_FILE=${DIR}/${uram_name}.${DATA_POSTFIX}
	read -a array < ${URAM_INFO_FILE}

	PID=${array[0]}
	URAM_PAGE_START_OFFSET=${array[2]}
	URAM_PAGE_COUNT=${array[3]}

	echo "#####################################"
	echo "Name of target device: ${uram_name}"
	echo "Latest PID of poseidonos: $PID"
	echo "Latest Logical page number in hugepage: $URAM_PAGE_START_OFFSET"
	echo "Page counts of data: $URAM_PAGE_COUNT"
	echo "Starting to backup URAM contents.."

	FIRST_BIN_FILE_NAME=/dev/hugepages/spdk_pid${PID}map_${URAM_PAGE_START_OFFSET}
	if [ ! -f $FIRST_BIN_FILE_NAME ]; then
		print_error ${uram_name}
	else
		sudo dd if="${FIRST_BIN_FILE_NAME}" of=${URAM_DATA_FILE} bs=2M count=1 status=none
		if [ $? -ne 0 ]; then
			print_error ${uram_name}
			continue
		fi

		FILE_OFFSET=1
		IO_SUCCESS=1
		for pageIndex in `seq $(($URAM_PAGE_START_OFFSET+1)) $(($URAM_PAGE_START_OFFSET+$URAM_PAGE_COUNT-1))`
		do
			FILE_NAME="/dev/hugepages/spdk_pid${PID}map_${pageIndex}"
			if [ ! -f ${FILE_NAME} ]; then
				print_error ${uram_name}
				IO_SUCCESS=0
				break
			fi
			sudo dd if="${FILE_NAME}" of=${URAM_DATA_FILE} bs=2M count=1 seek=$FILE_OFFSET conv=nocreat,notrunc  status=none &
			FILE_OFFSET=$((FILE_OFFSET+1))
		done
		wait

		if [ ${IO_SUCCESS} -eq 0 ]; then
			continue
		else
			echo -e ${GREEN_COLOR}${uram_name} backup has been completed!${RESET_COLOR}
		fi
	fi
done

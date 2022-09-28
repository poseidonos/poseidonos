#!/bin/bash

cd $(dirname $0)
DEVNAME=$1
ROOTDIR=../../
DEVBDF=0

get_bdf(){
    ${ROOTDIR}/bin/poseidonos-cli device list | grep $1 | awk -F'|' '{print $3}'
    echo $?
}

remove_device()
{
    echo "Remove Device Addr:" $1
    echo 1 > /sys/bus/pci/devices/$1/remove
}

pci_rescan()
{
    echo "Rescan Pci Devices"
    echo 1 > /sys/bus/pci/rescan
}

if [ $# -lt 2 ]; then
    echo "This script requires device name and rescan flag"
    echo "./detach_device.sh [Device Name] [Rescan Flag]"
    echo "EX) ./detach_device.sh unvme-ns-0 1"
else
    echo "Remove Device Name :" $DEVNAME
    DEVBDF=$(get_bdf $DEVNAME)
    if [ ${#DEVBDF} == 1 ];then
        echo "There is no device named : " $DEVNAME
        exit 1
    fi
    
    remove_device $DEVBDF

    if [ $2 -eq 1 ]; then
        sleep 2
        pci_rescan
    fi
    sleep 3
fi

exit 0

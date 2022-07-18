#!/bin/bash

# if device is busy, kill process
fuser -ck /etc/pos/POSRaid

# check md status
mdadm --detail /dev/md9
cat /proc/mdstat

# unmount 
umount /dev/md9

# stop construct RAID
mdadm --stop /dev/md9

# remove RAID construct
mdadm --zero-superblock /dev/nvme0n1
mdadm --zero-superblock /dev/nvme1n1

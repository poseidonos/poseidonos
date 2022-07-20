#!/bin/bash

echo "This script is only for testing rocksdb metafs on local environment"
echo "You can run this script in case only you do understand rocksmeta exactly"
echo "To continue script, enter any key"
read temp

echo "This script release RAID 1 devices (/dev/nvme0n1 and /dev/nvme1n1) which was used as RocksDB Meta File System"

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

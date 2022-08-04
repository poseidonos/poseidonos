#!/bin/bash

echo "This script is only for testing rocksdb metafs on local environment"
echo "You can run this script in case only you do understand rocksmeta exactly"
echo "To continue script, enter any key"
read temp

echo "This script release RAID 1 devices (/dev/nvme0n1 and /dev/nvme1n1) which was used as RocksDB Meta File System"

read -p "Enter rocksdb path : " rocksdbPath
read -p "Enter RAID device : " raidDev
read -p "Enter SSD first block device (ex. /dev/nvme0n1 ): " dev1
read -p "Enter SSD second block device (ex. /dev/nvme1n1 ): " dev2
# if device is busy, kill process
fuser -ck {rocksdbPath}

# check md status
mdadm --detail ${raidDev}
cat /proc/mdstat

# unmount 
umount ${raidDev}

# stop construct RAID
mdadm --stop ${raidDev}

# remove RAID construct
mdadm --zero-superblock ${dev1}
mdadm --zero-superblock ${dev2}

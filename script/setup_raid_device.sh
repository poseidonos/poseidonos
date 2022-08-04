#!/bin/bash

echo "This script is only for testing rocksdb metafs on local environment"
echo "You can run this script in case only you do understand rocksmeta exactly"
echo "To continue script, enter any key"
read temp

echo "You must run setup_env_rocksmeta.sh before running this script"
echo "This script construct RAID 1 with device /dev/nvme0n1 and /dev/nvme1n1 which will be used as RocksDB Meta File System"

read -p "Enter first device to construct RAID 1 (ex. /dev/nvme0n1), device must not have any partition : " dev1
read -p "Enter second device to construct RAID 1 (ex. /dev/nvme1n1), device must not have any partition : " dev2
read -p "Enter RAID device name (ex. /dev/md9) : " raidDev
echo "${dev1} and ${dev2} devices will construct RAID 1 ${raidDev} " 

# /dev/nvme0n1 and /dev/nvme1n1 must not have any partition before. 
# Make dev1 to raid type
(
    echo n # Add new partition
    echo p # Primary partition
    echo   # Partition number : use default
    echo   # First sector : use default
    echo   # Last sector : use default , sometimes select partition next line
    echo t # Change partition type
    echo fd # Change linux partition type to linux raid auto type
    echo w # Write changes
) | fdisk ${dev1}

# Make dev2 to raid type
(
    echo n # Add new partition
    echo p # Primary partition
    echo   # Partition number : use default
    echo   # First sector : use default
    echo   # Last sector : use default , sometimes select partition next line
    echo t # Change partition type
    echo fd # Change linux partition type to linux raid auto type
    echo w # Write changes
) | fdisk ${dev2}

# Construct RAID 1 with dev1 and dev2

(
    echo y # create array -> yes
) | mdadm --create ${raidDev} --level=1 --raid-devices=2 ${dev1} ${dev2}

# Format RAID device as xfs file system
(
    echo y 
) | mkfs.xfs ${raidDev}

# mount raid device to raidDev
read -p "Enter Rocksdb path in pos.conf (ex. /etc/pos/POSRaid ) : " rocksdbPath
mkdir ${rocksdbPath}
mount ${raidDev} ${rocksdbPath}

# check raid device is mounted
echo "ls RocksDB path command result"
ls ${rocksdbPath}

# check raid device 
mdadm --detail ${raidDev}

echo "Directory ${rocksdbPath} is created and constructed RAID 1 with ${dev1}, ${dev2}"

# Add raid permanently

# raidData=$( mdadm --detail --scan)
# echo "raid data is : $raidData"
# echo "$raidData" | tee -a /etc/mdadm/mdadm.conf

# 1. mdadm.conf update complete
# update-initramfs-u

# 2. mount complete
# mount_update="/dev/md9 /raidDir ext4 defaults 0 0"
# echo "$mount_update" | tee -a /etc/fstab
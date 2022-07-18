#!/bin/bash

# You must run setup_env_rocksmeta.sh before running this script

# Make /dev/nvme0n1 to raid type
(
    echo n # Add new partition
    echo p # Primary partition
    echo   # Partition number : use default
    echo   # First sector : use default
    echo   # Last sector : use default , sometimes select partition next line
    echo t # Change partition type
    echo fd # Change linux partition type to linux raid auto type
    echo w # Write changes
) | fdisk /dev/nvme0n1

# Construct RAID

(
    echo y # create array -> yes
) | mdadm --create /dev/md9 --level=1 --raid-devices=2 /dev/nvme0n1 /dev/nvme1n1

# Format RAID device as ext4 file system
(
    echo y 
) | mkfs.ext4 /dev/md9

# mount raid device to /raidDir
mkdir /etc/pos/POSRaid
mount /dev/md9 /etc/pos/POSRaid

# check raid device is mounted
ls /etc/pos/POSRaid

# check raid device 
mdadm --detail /dev/md9

echo "Directory /etc/pos/POSRaid is created and constructed RAID 1 with /dev/nvme0n1, /dev/nvme1n1"

# Add raid permanently

# raidData=$( mdadm --detail --scan)
# echo "raid data is : $raidData"
# echo "$raidData" | tee -a /etc/mdadm/mdadm.conf

# 1. mdadm.conf update complete
# update-initramfs-u

# 2. mount complete
# mount_update="/dev/md9 /raidDir ext4 defaults 0 0"
# echo "$mount_update" | tee -a /etc/fstab
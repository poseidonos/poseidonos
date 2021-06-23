# Poseidon OS

Poseidon OS (POS) is a light-weight storage OS that offers the best performance and valuable features over storage network. POS exploits the benefit of NVMe SSDs by optimizing storage stack and leveraging the state-of-the-art high speed interface. Please find project details at documentations page.


# Table of Contents
- [Source Code](#source-code)
- [How to Build](#how-to-build)
- [Preparation](#preparation)
- [Run POS](#run-pos)
- [Learning POS Commands](#learning-pos-commands)


## Source Code

```bash
git clone https://github.com/poseidonos/poseidonos.git

```

## Prerequisites

This script will automatically install the minimum dependencies required to build Poseidon OS.

```bash
cd script
sudo ./pkgdep.sh
```

## How to Build

### 1. Build Library

```bash
cd lib
sudo ./build_ibof_lib.sh all
```

### 2. Build Source Code

```bash
cd script
sudo ./build_ibofos.sh
```

## Preparation

```bash
cd script/
sudo ./setup_env.sh
```

## Run POS

```bash
sudo ./bin/ibofos
```

## Learning POS Commands

This document demonstrates how to start up POS and manage storage resources. The target audience is whoever wants to learn about POS and explore its capabilities. The minimum knowledge of Linux administration would be sufficient.

### Prerequisites

```bash
Hardware: Poseidon server
 - Reference server h/w implementation engineered by Samsung and Inspur
 - The number of processors: 2
 - The number of memory slots: 32
 - Memory speed: 3200 MT/s
 - Network speed: up to 600 GbE
 - PCIe generation: gen4
 - Storage: E1.S SSD * 32 ea
OS: Ubuntu 18.04 (kernel: 5.3.0-24-generic)
Application binary: ibofos, cli
Application config: /etc/ibofos/conf/ibofos.conf
Application scripts to configure environments
Writable file system for RPC socket, application logs/dump, and hugepage info
```

### Step 1. Start POS application

```bash
# Become a root user and check if you have local NVMe devices attached to the OS with its Kernel Device Driver.
ibof@R2U18-PSD-1-CI_TARGET:~$ su -
Password:
root@R2U18-PSD-1-CI_TARGET:~# cd /poseidonos
root@R2U18-PSD-1-CI_TARGET:/poseidonos# fdisk -l | grep nvme
Disk /dev/nvme0n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme1n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme2n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme5n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme6n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme12n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme15n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme9n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme10n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme17n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme18n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme16n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme22n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme26n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme23n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme25n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme20n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme19n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme8n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme3n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme7n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme4n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme28n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme29n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme11n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme13n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme31n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme14n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme24n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme21n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme27n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
Disk /dev/nvme30n1: 3.5 TiB, 3840755982336 bytes, 7501476528 sectors
 
# Start POS and check if the NVMe devices are detached from Linux kernel and attached to SPDK (user level application)
root@R2U18-PSD-1-CI_TARGET:/poseidonos# cd script/
root@R2U18-PSD-1-CI_TARGET:/poseidonos/script# ls -al
root@R2U18-PSD-1-CI_TARGET:/poseidonos/script# ./start_ibofos.sh
0000:4d:00.0 (144d a80a): Already using the nvme driver
0000:4e:00.0 (144d a80a): Already using the nvme driver
0000:4f:00.0 (144d a80a): Already using the nvme driver
0000:50:00.0 (144d a80a): Already using the nvme driver
0000:51:00.0 (144d a80a): Already using the nvme driver
0000:52:00.0 (144d a80a): Already using the nvme driver
0000:53:00.0 (144d a80a): Already using the nvme driver
0000:54:00.0 (144d a80a): Already using the nvme driver
0000:67:00.0 (144d a80a): Already using the nvme driver
0000:68:00.0 (144d a80a): Already using the nvme driver
0000:69:00.0 (144d a80a): Already using the nvme driver
0000:6a:00.0 (144d a80a): Already using the nvme driver
0000:6b:00.0 (144d a80a): Already using the nvme driver
0000:6c:00.0 (144d a80a): Already using the nvme driver
0000:6d:00.0 (144d a80a): Already using the nvme driver
0000:6e:00.0 (144d a80a): Already using the nvme driver
0000:cc:00.0 (144d a80a): Already using the nvme driver
0000:cd:00.0 (144d a80a): Already using the nvme driver
0000:ce:00.0 (144d a80a): Already using the nvme driver
0000:cf:00.0 (144d a80a): Already using the nvme driver
0000:d0:00.0 (144d a80a): Already using the nvme driver
0000:d1:00.0 (144d a80a): Already using the nvme driver
0000:d2:00.0 (144d a80a): Already using the nvme driver
0000:d3:00.0 (144d a80a): Already using the nvme driver
0000:e5:00.0 (144d a80a): Already using the nvme driver
0000:e6:00.0 (144d a80a): Already using the nvme driver
0000:e7:00.0 (144d a80a): Already using the nvme driver
0000:e8:00.0 (144d a80a): Already using the nvme driver
0000:e9:00.0 (144d a80a): Already using the nvme driver
0000:ea:00.0 (144d a80a): Already using the nvme driver
0000:eb:00.0 (144d a80a): Already using the nvme driver
0000:ec:00.0 (144d a80a): Already using the nvme driver
0000:00:01.0 (8086 0b00): Already using the ioatdma driver
0000:00:01.1 (8086 0b00): Already using the ioatdma driver
0000:00:01.2 (8086 0b00): Already using the ioatdma driver
0000:00:01.3 (8086 0b00): Already using the ioatdma driver
0000:00:01.4 (8086 0b00): Already using the ioatdma driver
0000:00:01.5 (8086 0b00): Already using the ioatdma driver
0000:00:01.6 (8086 0b00): Already using the ioatdma driver
0000:00:01.7 (8086 0b00): Already using the ioatdma driver
0000:80:01.0 (8086 0b00): Already using the ioatdma driver
0000:80:01.1 (8086 0b00): Already using the ioatdma driver
0000:80:01.2 (8086 0b00): Already using the ioatdma driver
0000:80:01.3 (8086 0b00): Already using the ioatdma driver
0000:80:01.4 (8086 0b00): Already using the ioatdma driver
0000:80:01.5 (8086 0b00): Already using the ioatdma driver
0000:80:01.6 (8086 0b00): Already using the ioatdma driver
0000:80:01.7 (8086 0b00): Already using the ioatdma driver
Setting maximum # of Huge Page Size is 128GB
0000:4d:00.0 (144d a80a): nvme -> uio_pci_generic
0000:4e:00.0 (144d a80a): nvme -> uio_pci_generic
0000:4f:00.0 (144d a80a): nvme -> uio_pci_generic
0000:50:00.0 (144d a80a): nvme -> uio_pci_generic
0000:51:00.0 (144d a80a): nvme -> uio_pci_generic
0000:52:00.0 (144d a80a): nvme -> uio_pci_generic
0000:53:00.0 (144d a80a): nvme -> uio_pci_generic
0000:54:00.0 (144d a80a): nvme -> uio_pci_generic
0000:67:00.0 (144d a80a): nvme -> uio_pci_generic
0000:68:00.0 (144d a80a): nvme -> uio_pci_generic
0000:69:00.0 (144d a80a): nvme -> uio_pci_generic
0000:6a:00.0 (144d a80a): nvme -> uio_pci_generic
0000:6b:00.0 (144d a80a): nvme -> uio_pci_generic
0000:6c:00.0 (144d a80a): nvme -> uio_pci_generic
0000:6d:00.0 (144d a80a): nvme -> uio_pci_generic
0000:6e:00.0 (144d a80a): nvme -> uio_pci_generic
0000:cc:00.0 (144d a80a): nvme -> uio_pci_generic
0000:cd:00.0 (144d a80a): nvme -> uio_pci_generic
0000:ce:00.0 (144d a80a): nvme -> uio_pci_generic
0000:cf:00.0 (144d a80a): nvme -> uio_pci_generic
0000:d0:00.0 (144d a80a): nvme -> uio_pci_generic
0000:d1:00.0 (144d a80a): nvme -> uio_pci_generic
0000:d2:00.0 (144d a80a): nvme -> uio_pci_generic
0000:d3:00.0 (144d a80a): nvme -> uio_pci_generic
0000:e5:00.0 (144d a80a): nvme -> uio_pci_generic
0000:e6:00.0 (144d a80a): nvme -> uio_pci_generic
0000:e7:00.0 (144d a80a): nvme -> uio_pci_generic
0000:e8:00.0 (144d a80a): nvme -> uio_pci_generic
0000:e9:00.0 (144d a80a): nvme -> uio_pci_generic
0000:ea:00.0 (144d a80a): nvme -> uio_pci_generic
0000:eb:00.0 (144d a80a): nvme -> uio_pci_generic
0000:ec:00.0 (144d a80a): nvme -> uio_pci_generic
0000:00:01.0 (8086 0b00): ioatdma -> uio_pci_generic
0000:00:01.1 (8086 0b00): ioatdma -> uio_pci_generic
0000:00:01.2 (8086 0b00): ioatdma -> uio_pci_generic
0000:00:01.3 (8086 0b00): ioatdma -> uio_pci_generic
0000:00:01.4 (8086 0b00): ioatdma -> uio_pci_generic
0000:00:01.5 (8086 0b00): ioatdma -> uio_pci_generic
0000:00:01.6 (8086 0b00): ioatdma -> uio_pci_generic
0000:00:01.7 (8086 0b00): ioatdma -> uio_pci_generic
0000:80:01.0 (8086 0b00): ioatdma -> uio_pci_generic
0000:80:01.1 (8086 0b00): ioatdma -> uio_pci_generic
0000:80:01.2 (8086 0b00): ioatdma -> uio_pci_generic
0000:80:01.3 (8086 0b00): ioatdma -> uio_pci_generic
0000:80:01.4 (8086 0b00): ioatdma -> uio_pci_generic
0000:80:01.5 (8086 0b00): ioatdma -> uio_pci_generic
0000:80:01.6 (8086 0b00): ioatdma -> uio_pci_generic
0000:80:01.7 (8086 0b00): ioatdma -> uio_pci_generic
/home/ibof/projects/poseidonos/script
apport.service is not a native service, redirecting to systemd-sysv-install.
Executing: /lib/systemd/systemd-sysv-install disable apport
Current maximum # of memory map areas per process is 65530.
Setting maximum # of memory map areas per process to 65535.
vm.max_map_count = 65535
Setup env. done!
Execute ibofos
Wait ibofos
ibofos is running in background...logfile=ibofos.log
 
# Verify if the application is up and running
root@R2U18-PSD-1-CI_TARGET:/poseidonos/script# ps -ef | grep ibofos | grep -v grep
root     78059     1 99 03:01 pts/0    00:02:53 /home/ibof/projects/poseidonos/script/..//bin/ibofos
 
# Unlike in the the previous execution, you shouldn't see the NVMe devices from the fdisk output since all of them must have been reattached from OS to SPDK.
root@R2U18-PSD-1-CI_TARGET:/poseidonos/script# fdisk -l |grep nvme
```

### Step 2. Create Write Buffer within DRAM

```bash
root@R2U18-PSD-1-CI_TARGET:/poseidonos/script# cd ../lib/spdk-19.10/scripts/
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts#
 
# Check the usage to create write buffer for POS array
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# ./rpc.py bdev_malloc_create -h
usage: rpc.py bdev_malloc_create [-h] [-b NAME] [-u UUID]
                                 total_size block_size
 
positional arguments:
  total_size            Size of malloc bdev in MB (float > 0)
  block_size            Block size for this bdev
 
optional arguments:
  -h, --help            show this help message and exit
  -b NAME, --name NAME  Name of the bdev
  -u UUID, --uuid UUID  UUID of the bdev
 
# Create write buffer with the total size of 8192 MB, the block size of 512 B, and the name of "uram0".
# Technically, the command is sent to SPDK server and creates a SPDK block device called "malloc bdev", which is a userspace ramdisk.
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# ./rpc.py bdev_malloc_create -b uram0 8192 512
uram0
```

```bash
The recommended size of uram0 may differ by environment. Please refer to "bdev" section in Learning POS Environment for further details.
```

### Step 3. Check POS information and version

```bash
# You don't have to become root to run "cli" commands. The actual output may differ by env where the command is executed.
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# logout
ibof@R2U18-PSD-1-CI_TARGET:~$ cd /poseidonos/bin/
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli system info
 
 
Request to Poseidon OS
    xrId        :  333e8518-3243-11ea-b47f-a0369f78dd8c
    command     :  GETIBOFOSINFO
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "capacity": "0GB (0B)",
    "rebuildingProgress": "0",
    "situation": "DEFAULT",
    "state": "OFFLINE",
    "used": "0GB (0B)"
}
 
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli system version
 
 
Request to Poseidon OS
    xrId        :  61a8531e-3243-11ea-a489-a0369f78dd8c
    command     :  GETVERSION
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
{
    "version": "pos-0.7.3-alpha8"
}
```

### Step 4. Scan NVMe Devices

```bash
# If this is the first run, you wouldn't see any devices showing up in the output as in the following.
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli device list
 
 
Request to Poseidon OS
    xrId        :  a9a36672-3243-11ea-ad67-a0369f78dd8c
    command     :  LISTDEVICE
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
 
# Issue "scan" command so that POS can discover NVMe devices and refresh its device information
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli device scan
 
 
Request to Poseidon OS
    xrId        :  d5c9e3a6-3243-11ea-be3b-a0369f78dd8c
    command     :  SCANDEVICE
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
 
  
# Let's give it one more try with "list". Now that local NVMe devices have been scanned, POS should
# be able to give us the list.
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli device list
 
 
Request to Poseidon OS
    xrId        :  1269f0fe-3244-11ea-b75f-a0369f78dd8c
    command     :  LISTDEVICE
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "devicelist": [
        {
            "addr": "0000:4d:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-0",
            "size": 3840755982336,
            "sn": "FL10035M045141      ",
            "type": "SSD"
        },
        {
            "addr": "0000:4e:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-1",
            "size": 3840755982336,
            "sn": "FL10035M045142      ",
            "type": "SSD"
        },
        {
            "addr": "0000:4f:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-2",
            "size": 3840755982336,
            "sn": "FL10035M045126      ",
            "type": "SSD"
        },
        {
            "addr": "0000:50:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-3",
            "size": 3840755982336,
            "sn": "FL10035M045123      ",
            "type": "SSD"
        },
        {
            "addr": "0000:51:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-4",
            "size": 3840755982336,
            "sn": "FL10035M045121      ",
            "type": "SSD"
        },
        {
            "addr": "0000:52:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-5",
            "size": 3840755982336,
            "sn": "FL10035M045119      ",
            "type": "SSD"
        },
        {
            "addr": "0000:53:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-6",
            "size": 3840755982336,
            "sn": "FL10035M045128      ",
            "type": "SSD"
        },
        {
            "addr": "0000:54:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-7",
            "size": 3840755982336,
            "sn": "FL10035M045129      ",
            "type": "SSD"
        },
        {
            "addr": "0000:67:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-8",
            "size": 3840755982336,
            "sn": "FL10035M045113      ",
            "type": "SSD"
        },
        {
            "addr": "0000:68:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-9",
            "size": 3840755982336,
            "sn": "FL10035M045115      ",
            "type": "SSD"
        },
        {
            "addr": "0000:69:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-10",
            "size": 3840755982336,
            "sn": "FL10035M045117      ",
            "type": "SSD"
        },
        {
            "addr": "0000:6a:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-11",
            "size": 3840755982336,
            "sn": "FL10035M045133      ",
            "type": "SSD"
        },
        {
            "addr": "0000:6b:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-12",
            "size": 3840755982336,
            "sn": "FL10035M045135      ",
            "type": "SSD"
        },
        {
            "addr": "0000:6c:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-13",
            "size": 3840755982336,
            "sn": "FL10035M045136      ",
            "type": "SSD"
        },
        {
            "addr": "0000:6d:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-14",
            "size": 3840755982336,
            "sn": "FL10035M045140      ",
            "type": "SSD"
        },
        {
            "addr": "0000:6e:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-15",
            "size": 3840755982336,
            "sn": "FL10035M045137      ",
            "type": "SSD"
        },
        {
            "addr": "0000:cc:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-16",
            "size": 3840755982336,
            "sn": "FL10035M045139      ",
            "type": "SSD"
        },
        {
            "addr": "0000:cd:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-17",
            "size": 3840755982336,
            "sn": "FL10035M045144      ",
            "type": "SSD"
        },
        {
            "addr": "0000:ce:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-18",
            "size": 3840755982336,
            "sn": "FL10035M045132      ",
            "type": "SSD"
        },
        {
            "addr": "0000:cf:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-19",
            "size": 3840755982336,
            "sn": "FL10035M045131      ",
            "type": "SSD"
        },
        {
            "addr": "0000:d0:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-20",
            "size": 3840755982336,
            "sn": "FL10035M045138      ",
            "type": "SSD"
        },
        {
            "addr": "0000:d1:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-21",
            "size": 3840755982336,
            "sn": "FL10035M045134      ",
            "type": "SSD"
        },
        {
            "addr": "0000:d2:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-22",
            "size": 3840755982336,
            "sn": "FL10035M045114      ",
            "type": "SSD"
        },
        {
            "addr": "0000:d3:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-23",
            "size": 3840755982336,
            "sn": "FL10035M045130      ",
            "type": "SSD"
        },
        {
            "addr": "0000:e5:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-24",
            "size": 3840755982336,
            "sn": "FL10035M045127      ",
            "type": "SSD"
        },
        {
            "addr": "0000:e6:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-25",
            "size": 3840755982336,
            "sn": "FL10035M045120      ",
            "type": "SSD"
        },
        {
            "addr": "0000:e7:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-26",
            "size": 3840755982336,
            "sn": "FL10035M045118      ",
            "type": "SSD"
        },
        {
            "addr": "0000:e8:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-27",
            "size": 3840755982336,
            "sn": "FL10035M045122      ",
            "type": "SSD"
        },
        {
            "addr": "0000:e9:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-28",
            "size": 3840755982336,
            "sn": "FL10035M045124      ",
            "type": "SSD"
        },
        {
            "addr": "0000:ea:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-29",
            "size": 3840755982336,
            "sn": "FL10035M045125      ",
            "type": "SSD"
        },
        {
            "addr": "0000:eb:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-30",
            "size": 3840755982336,
            "sn": "FL10035M045116      ",
            "type": "SSD"
        },
        {
            "addr": "0000:ec:00.0",
            "class": "SYSTEM",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-31",
            "size": 3840755982336,
            "sn": "FL10035M045143      ",
            "type": "SSD"
        },
        {
            "addr": "",
            "class": "SYSTEM",
            "mn": "uram0",
            "name": "uram0",
            "size": 8589934592,
            "sn": "uram0",
            "type": "NVRAM"
        }
    ]
}
```

### Step 5. Import POS Array

If you have not created any POS array before, you could start with a new one and import into the POS (Step 5a). Otherwise, you could load existing POS arrays if POS is not aware of them due to host reboot or POS restart (Step 5b). 

### 5a. Create new POS Array

Now that POS has completed the scanning, it should be able to create POS array with a set of block devices we choose.

```bash
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli array create -b uram0 -d unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3,unvme-ns-4,unvme-ns-5,unvme-ns-6,unvme-ns-7,unvme-ns-8,unvme-ns-9,unvme-ns-10,unvme-ns-11,unvme-ns-12,unvme-ns-13,unvme-ns-14,unvme-ns-15,unvme-ns-16,unvme-ns-17,unvme-ns-18,unvme-ns-19,unvme-ns-20,unvme-ns-21,unvme-ns-22,unvme-ns-23,unvme-ns-24,unvme-ns-25,unvme-ns-26,unvme-ns-27,unvme-ns-28 -s unvme-ns-29,unvme-ns-30,unvme-ns-31 --name POSArray --raidtype RAID5
 
 
Request to Poseidon OS
    xrId        :  4f249204-3246-11ea-97df-a0369f78dd8c
    command     :  CREATEARRAY
    Param       :
{
    "name": "POSArray",
    "raidtype": "RAID5",
    "buffer": [
        {
            "deviceName": "uram0"
        }
    ],
    "data": [
        {
            "deviceName": "unvme-ns-0"
        },
        {
            "deviceName": "unvme-ns-1"
        },
        {
            "deviceName": "unvme-ns-2"
        },
        {
            "deviceName": "unvme-ns-3"
        },
        {
            "deviceName": "unvme-ns-4"
        },
        {
            "deviceName": "unvme-ns-5"
        },
        {
            "deviceName": "unvme-ns-6"
        },
        {
            "deviceName": "unvme-ns-7"
        },
        {
            "deviceName": "unvme-ns-8"
        },
        {
            "deviceName": "unvme-ns-9"
        },
        {
            "deviceName": "unvme-ns-10"
        },
        {
            "deviceName": "unvme-ns-11"
        },
        {
            "deviceName": "unvme-ns-12"
        },
        {
            "deviceName": "unvme-ns-13"
        },
        {
            "deviceName": "unvme-ns-14"
        },
        {
            "deviceName": "unvme-ns-15"
        },
        {
            "deviceName": "unvme-ns-16"
        },
        {
            "deviceName": "unvme-ns-17"
        },
        {
            "deviceName": "unvme-ns-18"
        },
        {
            "deviceName": "unvme-ns-19"
        },
        {
            "deviceName": "unvme-ns-20"
        },
        {
            "deviceName": "unvme-ns-21"
        },
        {
            "deviceName": "unvme-ns-22"
        },
        {
            "deviceName": "unvme-ns-23"
        },
        {
            "deviceName": "unvme-ns-24"
        },
        {
            "deviceName": "unvme-ns-25"
        },
        {
            "deviceName": "unvme-ns-26"
        },
        {
            "deviceName": "unvme-ns-27"
        },
        {
            "deviceName": "unvme-ns-28"
        }
    ],
    "spare": [
        {
            "deviceName": "unvme-ns-29"
        },
        {
            "deviceName": "unvme-ns-30"
        },
        {
            "deviceName": "unvme-ns-31"
        }
    ]
}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
```

As you may have noticed, some of the parameters should be passed in from the output of the previous step ("cli device list" command).
 - The write buffer device (-b) should be the name of a device whose "type" is "NVRAM".
 - The data devices (-d) should be comma-separated list of devices, the type of which being "SSD".
 - The name of POS array must comply with a naming convention described in Creating POS Array. 
 - As of Nov/30/2020, the available RAID types (--raidtype) are only ["RAID5"]

Once POS array has been created, you could query the POS array information as in the following:

```bash
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli array list_device --name POSArray
 
 
Request to Poseidon OS
    xrId        :  b5c0f89c-3246-11ea-a125-a0369f78dd8c
    command     :  LISTARRAYDEVICE
    Param       :
{
    "name": "POSArray"
}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "devicelist": [
        {
            "name": "uram0",
            "type": "BUFFER"
        },
        {
            "name": "unvme-ns-0",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-1",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-2",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-3",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-4",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-5",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-6",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-7",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-8",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-9",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-10",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-11",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-12",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-13",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-14",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-15",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-16",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-17",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-18",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-19",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-20",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-21",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-22",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-23",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-24",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-25",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-26",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-27",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-28",
            "type": "DATA"
        },
        {
            "name": "unvme-ns-29",
            "type": "SPARE"
        },
        {
            "name": "unvme-ns-30",
            "type": "SPARE"
        },
        {
            "name": "unvme-ns-31",
            "type": "SPARE"
        }
    ]
}
```

### Step 5b. Load existing POS Array

"Load" command is to support a case that POS/host has restarted and lost its in-memory state. The following command will retrieve the array information from MBR partition and import into POS. 

```bash
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli array load --name POSArray
 
 
Request to Poseidon OS
    xrId        :  f4b6f898-3246-11ea-b74e-a0369f78dd8c
    command     :  LOADARRAY
    Param       :
{
    "name": "POSArray"
}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
```

Please make sure that Step 2 (creating write buffer) and Step 4 (scanning NVMe devices) should run before the LOADARRAY command. After a reboot/restart, the write buffer should be recreated and rescanned. Otherwise, LOADARRAY command would fail. 


### Step 6. Mount POS Array 
Even though we have POS array provisioned, we can't use it until it is mounted. Let's check out what happens with system state around POS array mount.

```bash
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli system info
 
 
Request to Poseidon OS
    xrId        :  30b300b0-3247-11ea-8e5c-a0369f78dd8c
    command     :  GETIBOFOSINFO
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "capacity": "0GB (0B)",
    "rebuildingProgress": "0",
    "situation": "DEFAULT",
    "state": "OFFLINE",
    "used": "0GB (0B)"
}
 
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli system mount
 
 
Request to Poseidon OS
    xrId        :  3dda457a-3247-11ea-b7a0-a0369f78dd8c
    command     :  MOUNTIBOFOS
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
 
  
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli system info
 
 
Request to Poseidon OS
    xrId        :  4bd07a89-3247-11ea-ac3d-a0369f78dd8c
    command     :  GETIBOFOSINFO
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "capacity": "94.832555773133TB (94832555773133B)",
    "rebuildingProgress": "0",
    "situation": "NORMAL",
    "state": "NORMAL",
    "used": "0GB (0B)"
}
```

Please note that state field in the output has changed from OFFLINE to NORMAL. Also, "capacity" is now reflecting the size of the NVMe storage pool available to POS. 
```bash
Please note that, as of Nov/30/2020, POS supports a single POS array only, which is why "mount" command belongs to "system" (i.e., it's currently "cli system mount", but not "cli array mount --name POSArray"). Once POS gets a new feature to support multi array, the command will change accordingly. 
```

### Step 7. Configure NVM Subsystems for NVMe Over Fabric Target
POS is ready to perform volume management task, but still unable to expose its volume over network since we haven't configured an NVM subsystem yet. POS is not ready to expose its volume over network since it does not have NVM subsystem in which NVM namespaces(s) are created. Creating NVM subsystem remains in manual fashion  (vs. running automatically during POS startup) by design. Administrators need to understand its functionality so that they can easily come up with a workaround when needed. Once we have enough understanding about various user environments, this step could be automated in a future release.

Create NVMe-oF Subsystem
```bash
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ su -
Password:
root@R2U18-PSD-1-CI_TARGET:~# cd /poseidonos/lib/spdk-19.10/scripts/
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# ./rpc.py nvmf_create_subsystem -h
usage: rpc.py nvmf_create_subsystem [-h] [-t TGT_NAME] [-s SERIAL_NUMBER]
                                    [-d MODEL_NUMBER] [-a] [-m MAX_NAMESPACES]
                                    nqn
 
positional arguments:
  nqn                   Subsystem NQN (ASCII)
 
optional arguments:
  -h, --help            show this help message and exit
  -t TGT_NAME, --tgt_name TGT_NAME
                        The name of the parent NVMe-oF target (optional)
  -s SERIAL_NUMBER, --serial-number SERIAL_NUMBER
                        Format: 'sn' etc Example: 'SPDK00000000000001'
  -d MODEL_NUMBER, --model-number MODEL_NUMBER
                        Format: 'mn' etc Example: 'SPDK Controller'
  -a, --allow-any-host  Allow any host to connect (don't enforce host NQN
                        whitelist)
  -m MAX_NAMESPACES, --max-namespaces MAX_NAMESPACES
                        Maximum number of namespaces allowed
  
 
# If successful, the following doesn't print out any response
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# ./rpc.py nvmf_create_subsystem nqn.2019-04.ibof:subsystem1 -a -s IBOF00000000000001 -d IBOF_VOLUME_EXTENSION -m 256
```

The following command configures TCP transport to use when network connection is established between an initiator and a target. is between initiator and target. 

Create NVMe-oF Transport
```bash
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# ./rpc.py nvmf_create_transport -h
usage: rpc.py nvmf_create_transport [-h] -t TRTYPE [-g TGT_NAME]
                                    [-q MAX_QUEUE_DEPTH]
                                    [-p MAX_QPAIRS_PER_CTRLR]
                                    [-c IN_CAPSULE_DATA_SIZE] [-i MAX_IO_SIZE]
                                    [-u IO_UNIT_SIZE] [-a MAX_AQ_DEPTH]
                                    [-n NUM_SHARED_BUFFERS]
                                    [-b BUF_CACHE_SIZE] [-s MAX_SRQ_DEPTH]
                                    [-r] [-o] [-f] [-y SOCK_PRIORITY]
 
optional arguments:
  -h, --help            show this help message and exit
  -t TRTYPE, --trtype TRTYPE
                        Transport type (ex. RDMA)
  -g TGT_NAME, --tgt_name TGT_NAME
                        The name of the parent NVMe-oF target (optional)
  -q MAX_QUEUE_DEPTH, --max-queue-depth MAX_QUEUE_DEPTH
                        Max number of outstanding I/O per queue
  -p MAX_QPAIRS_PER_CTRLR, --max-qpairs-per-ctrlr MAX_QPAIRS_PER_CTRLR
                        Max number of SQ and CQ per controller
  -c IN_CAPSULE_DATA_SIZE, --in-capsule-data-size IN_CAPSULE_DATA_SIZE
                        Max number of in-capsule data size
  -i MAX_IO_SIZE, --max-io-size MAX_IO_SIZE
                        Max I/O size (bytes)
  -u IO_UNIT_SIZE, --io-unit-size IO_UNIT_SIZE
                        I/O unit size (bytes)
  -a MAX_AQ_DEPTH, --max-aq-depth MAX_AQ_DEPTH
                        Max number of admin cmds per AQ
  -n NUM_SHARED_BUFFERS, --num-shared-buffers NUM_SHARED_BUFFERS
                        The number of pooled data buffers available to the
                        transport
  -b BUF_CACHE_SIZE, --buf-cache-size BUF_CACHE_SIZE
                        The number of shared buffers to reserve for each poll
                        group
  -s MAX_SRQ_DEPTH, --max-srq-depth MAX_SRQ_DEPTH
                        Max number of outstanding I/O per SRQ. Relevant only
                        for RDMA transport
  -r, --no-srq          Disable per-thread shared receive queue. Relevant only
                        for RDMA transport
  -o, --c2h-success     Disable C2H success optimization. Relevant only for
                        TCP transport
  -f, --dif-insert-or-strip
                        Enable DIF insert/strip. Relevant only for TCP
                        transport
  -y SOCK_PRIORITY, --sock-priority SOCK_PRIORITY
                        The sock priority of the tcp connection. Relevant only
                        for TCP transport
 
 
# If successful, the following doesn't print out any response
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# ./rpc.py nvmf_create_transport -t tcp -b 64 -n 4096
```

The following command makes a given NVM subsystem listen on a TCP port and serve incoming NVMe-oF requests. 

Add NVMe-oF Subsystem Listener
```bash
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# ./rpc.py nvmf_subsystem_add_listener -h
usage: rpc.py nvmf_subsystem_add_listener [-h] -t TRTYPE -a TRADDR
                                          [-p TGT_NAME] [-f ADRFAM]
                                          [-s TRSVCID]
                                          nqn
 
positional arguments:
  nqn                   NVMe-oF subsystem NQN
 
optional arguments:
  -h, --help            show this help message and exit
  -t TRTYPE, --trtype TRTYPE
                        NVMe-oF transport type: e.g., rdma
  -a TRADDR, --traddr TRADDR
                        NVMe-oF transport address: e.g., an ip address
  -p TGT_NAME, --tgt_name TGT_NAME
                        The name of the parent NVMe-oF target (optional)
  -f ADRFAM, --adrfam ADRFAM
                        NVMe-oF transport adrfam: e.g., ipv4, ipv6, ib, fc,
                        intra_host
  -s TRSVCID, --trsvcid TRSVCID
                        NVMe-oF transport service id: e.g., a port number
 
# Check out what NICs are available on this host
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# ifconfig
ens21f0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 10.1.2.18  netmask 255.255.0.0  broadcast 10.1.255.255
        inet6 fe80::d2e7:8941:fa2c:d4c6  prefixlen 64  scopeid 0x20<link>
        ether a0:36:9f:78:dd:8c  txqueuelen 1000  (Ethernet)
        RX packets 1438408  bytes 1736122845 (1.7 GB)
        RX errors 0  dropped 230  overruns 0  frame 0
        TX packets 714766  bytes 56404958 (56.4 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
 
ens21f1: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether a0:36:9f:78:dd:8e  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
 
ens5f0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        ether 04:3f:72:9b:ed:4a  txqueuelen 1000  (Ethernet)
        RX packets 11568  bytes 4006229 (4.0 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
 
ens5f1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 9000
        inet 10.100.2.18  netmask 255.255.255.0  broadcast 10.100.2.255
        inet6 fe80::f25d:f1ad:4648:f946  prefixlen 64  scopeid 0x20<link>
        ether 04:3f:72:9b:ed:4b  txqueuelen 1000  (Ethernet)
        RX packets 16567  bytes 4864266 (4.8 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 444  bytes 39889 (39.8 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
 
lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 83194  bytes 5964948 (5.9 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 83194  bytes 5964948 (5.9 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
 
# Pick up one of the NICs (e.g., ens21f0) and pass it to -a option. If successful, the following doesn't print out any response
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# ./rpc.py nvmf_subsystem_add_listener nqn.2019-04.ibof:subsystem1 -t tcp -a 10.1.2.18 -s 1158
```
In the above example, the NVM subsystem called "nqn.2019-04.ibof:subsystem1" has been configured to listen on (10.100.11.20, 1158) and use TCP transport. If you miss this step, POS wouldn't be able to mount POS volumes even though it could create new ones. 
At this point, you should be able to retrieve the configured NVM subsystem like in the following:

Retrieve NVM subsystem information
```bash
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# ./rpc.py nvmf_get_subsystems
[
  {
    "nqn": "nqn.2014-08.org.nvmexpress.discovery",
    "subtype": "Discovery",
    "listen_addresses": [],
    "allow_any_host": true,
    "hosts": []
  },
  {
    "nqn": "nqn.2019-04.ibof:subsystem1",
    "subtype": "NVMe",
    "listen_addresses": [
      {
        "transport": "TCP",
        "trtype": "TCP",
        "adrfam": "IPv4",
        "traddr": "10.1.2.18",
        "trsvcid": "1158"
      }
    ],
    "allow_any_host": true,
    "hosts": [],
    "serial_number": "IBOF00000000000001",
    "model_number": "IBOF_VOLUME_EXTENSION",
    "max_namespaces": 256,
    "namespaces": []
  }
]
```

### Step 8. Create POS Volume

This step is to create a logical entry point of the target side IO which will be shown as a namespace in an NVM subsystem. POS volume is wrapped as a "bdev" and can be attached to a particular NVM subsystem. "bdev" is a block device abstraction offered by SPDK library. 

Create a volume
```bash
# Create a 50-TB volume
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# logout
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli volume create --name vol1 --size 54975581388800 --maxiops 0 --maxbw 0 --array POSArray
 
 
Request to Poseidon OS
    xrId        :  7a693202-327b-11ea-9906-a0369f78dd8c
    command     :  CREATEVOLUME
    Param       :
{
    "name": "vol1",
    "array": "POSArray",
    "size": 54975581388800
}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
```

Retrieve volume information
```bash
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli volume create --name vol1 --size 54975581388800 --maxiops 0 --maxbw 0 --array POSArray
 
 
Request to Poseidon OS
    xrId        :  7a693202-327b-11ea-9906-a0369f78dd8c
    command     :  CREATEVOLUME
    Param       :
{
    "name": "vol1",
    "array": "POSArray",
    "size": 54975581388800
}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
```
Please note that the initial status of POS volume is Unmounted. 

### Step 9. Mount POS Volume

This is to make a particular POS volume ready to perform IO. After this step, POS volume is attached as bdev to an NVM subsystem and seen as an NVM namespace. 
```bash
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli volume mount --name vol1 --array POSArray
 
 
Request to Poseidon OS
    xrId        :  14e7d33a-327c-11ea-a8d0-a0369f78dd8c
    command     :  MOUNTVOLUME
    Param       :
{
    "name": "vol1",
    "array": "POSArray"
}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
 
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli volume list --array POSArray
 
 
Request to Poseidon OS
    xrId        :  6cff1f6a-327c-11ea-a9c8-a0369f78dd8c
    command     :  LISTVOLUME
    Param       :
{
    "array": "POSArray"
}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "volumes": [
        {
            "id": 0,
            "maxbw": 0,
            "maxiops": 0,
            "name": "vol1",
            "remain": "54.9755813888TB (54975581388800B)",
            "status": "Mounted",
            "total": "54.9755813888TB (54975581388800B)"
        }
    ]
}
 
  
  
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "volumes": [
        {
            "id": 0,
            "maxbw": 0,
            "maxiops": 0,
            "name": "vol1",
            "remain": "1.073741824GB (1073741824B)",
            "status": "Mounted",
            "total": "1.073741824GB (1073741824B)"
        }
    ]
}
```

Please note that the status of the volume has become Mounted.  If we check the NVM subsystem again, we can notice an NVM namespace has been added to an NVM subsystem with its bdev_name as follows.

Retrieve NVM subsystem information
```bash
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# ./rpc.py nvmf_get_subsystems
[
  {
    "nqn": "nqn.2014-08.org.nvmexpress.discovery",
    "subtype": "Discovery",
    "listen_addresses": [],
    "allow_any_host": true,
    "hosts": []
  },
  {
    "nqn": "nqn.2019-04.ibof:subsystem1",
    "subtype": "NVMe",
    "listen_addresses": [
      {
        "transport": "TCP",
        "trtype": "TCP",
        "adrfam": "IPv4",
        "traddr": "10.1.2.18",
        "trsvcid": "1158"
      }
    ],
    "allow_any_host": true,
    "hosts": [],
    "serial_number": "IBOF00000000000001",
    "model_number": "IBOF_VOLUME_EXTENSION",
    "max_namespaces": 256,
    "namespaces": [
      {
        "nsid": 1,
        "bdev_name": "bdev0",
        "name": "bdev0",
        "uuid": "d3d3dcda-1134-4f1d-979f-0f58168ed228"
      }
    ]
  }
]
```

```bash
Once mounted, the connection is established between an initiator and an NVM subsystem. Then, POS volume becomes accessible over network by an initiator.
```

### Step 10. Unmount POS Volume
```bash
root@R2U18-PSD-1-CI_TARGET:/poseidonos/lib/spdk-19.10/scripts# logout
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli volume unmount --name vol1 --array POSArray
 
 
Request to Poseidon OS
    xrId        :  cb218e5b-327c-11ea-a417-a0369f78dd8c
    command     :  UNMOUNTVOLUME
    Param       :
{
    "name": "vol1",
    "array": "POSArray"
}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
 
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli volume list --array POSArray
 
 
Request to Poseidon OS
    xrId        :  d8cb0eb9-327c-11ea-84df-a0369f78dd8c
    command     :  LISTVOLUME
    Param       :
{
    "array": "POSArray"
}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "volumes": [
        {
            "id": 0,
            "maxbw": 0,
            "maxiops": 0,
            "name": "vol1",
            "status": "Unmounted",
            "total": "54.9755813888TB (54975581388800B)"
        }
    ]
}
```
```bash
Please note that the status of the POS volume has changed from "Mounted" to "Unmounted".
```

### Step 11. Delete POS Volume
```bash
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli volume delete --name vol1 --array POSArray
 
 
Request to Poseidon OS
    xrId        :  e74c6df5-327c-11ea-9250-a0369f78dd8c
    command     :  DELETEVOLUME
    Param       :
{
    "name": "vol1",
    "array": "POSArray"
}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
 
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli volume list --array POSArray
 
 
Request to Poseidon OS
    xrId        :  001764a5-327d-11ea-ad8c-a0369f78dd8c
    command     :  LISTVOLUME
    Param       :
{
    "array": "POSArray"
}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
```
POS volume can be deleted only when it is in Unmounted state. 

### Step 12. Unmount POS Array
```bash
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli system unmount
 
 
Request to Poseidon OS
    xrId        :  10e208d4-327d-11ea-aba8-a0369f78dd8c
    command     :  UNMOUNTIBOFOS
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
 
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli system info
 
 
Request to Poseidon OS
    xrId        :  1dee075d-327d-11ea-ab6a-a0369f78dd8c
    command     :  GETIBOFOSINFO
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "capacity": "0GB (0B)",
    "rebuildingProgress": "0",
    "situation": "DEFAULT",
    "state": "OFFLINE",
    "used": "0GB (0B)"
}
```

### Step 13. Delete POS Array
```bash
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli array delete --name POSArray
 
 
Request to Poseidon OS
    xrId        :  2e9c9326-327d-11ea-9a3f-a0369f78dd8c
    command     :  DELETEARRAY
    Param       :
{
    "name": "POSArray"
}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
 
 
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli array info --name POSArray
 
 
Request to Poseidon OS
    xrId        :  39fdd5ec-327d-11ea-9b62-a0369f78dd8c
    command     :  ARRAYINFO
    Param       :
{
    "name": "POSArray"
}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "name": "POSArray",
    "state": "NOT_EXIST"
}
```
POS array can be deleted only when it is in OFFLINE state.

### Step 14. Shut down POS application
```bash
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ./cli system exit
 
 
Request to Poseidon OS
    xrId        :  4587f79d-327d-11ea-b9c8-a0369f78dd8c
    command     :  EXITIBOFOS
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
```
Please make sure that POS is not running anymore as in the following.
```bash
ibof@R2U18-PSD-1-CI_TARGET:/poseidonos/bin$ ps -ef | grep ibofos | grep -v grep
```



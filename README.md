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
sudo ./build_lib.sh
```

### 2. Build Source Code

```bash
cd script/
sudo ./build_ibofos.sh
```
## Run POS

```bash
cd script/
sudo ./start_poseidonos.sh
```

## Learning POS Commands

This document demonstrates how to start up POS and manage storage resources. The target audience is whoever wants to learn about POS and explore its capabilities. The minimum knowledge of Linux administration would be sufficient.

### Environments
The following configs are used for this demonstration, but may change as this document gets revised.
| Config | Value |
| ------ | ----- |
| OS     | Ubuntu 5.3.0-24-generic      |
| POS location | $POS_HOME/bin/poseidonos <br/> $POS_HOME/bin/cli |
| POS config | /etc/pos/pos.conf |
| POS scripts |  $POS_HOME/script/start_poseidonos.sh <br/> $POS_HOME/lib/spdk-20.10/script/rpc.py <br/> $POS_HOME/test/script/set_irq_affinity_cpulist.sh <br/> $POS_HOME/test/script/common_irq_affinity.sh <br/> $POS_HOME/script/setup_env.sh <br/> $POS_HOME/lib/spdk-20.10/scripts/setup.sh <br/> $POS_HOME/lib/spdk-20.10/scripts/common.sh |
| POS log | /var/log/pos/pos.log |
| POS dump | /etc/pos/core/poseidonos.core |
| SPDK RPC Server UDS | /var/tmp/spdk.sock |
| Hugepage information | /tmp/uram_hugepage |

The following hardware has been used for evaluation.
```bash
Hardware: Poseidon server
 - Reference server h/w implementation engineered by Samsung and Inspur
 - The number of processors: 2
 - The number of memory slots: 32
 - Memory speed: 3200 MT/s
 - Network speed: up to 600 GbE
 - PCIe generation: gen4
 - Storage: E1.S SSD * 32 ea
```
We provide the step-by-step guide to run POS commands with actual outputs. In the example, $POS_HOME is set to "/poseidonos" and the host name to "R2U14-PSD-3". The same could be achieved through web interface called M-Tool. Managing POS with M-Tool is explained in GUI section.

### Step 1. Start POS application

```bash
# Become a root user and check if you have local NVMe devices attached to the OS with its Kernel Device Driver.
ibof@R2U14-PSD-3:~$ su -
Password:
root@R2U14-PSD-3:~# cd /poseidonos
root@R2U14-PSD-3:/poseidonos# fdisk -l | grep nvme
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
root@R2U14-PSD-3:/poseidonos# cd script/
root@R2U14-PSD-3:/poseidonos/script# ls -al
root@R2U14-PSD-3:/poseidonos/script# ./start_poseidonos.sh
0000:4e:00.0 (144d a80a): uio_pci_generic -> nvme
0000:ce:00.0 (144d a80a): uio_pci_generic -> nvme
0000:ea:00.0 (144d a80a): uio_pci_generic -> nvme
0000:68:00.0 (144d a80a): uio_pci_generic -> nvme
0000:4d:00.0 (144d a80a): uio_pci_generic -> nvme
0000:50:00.0 (144d a80a): uio_pci_generic -> nvme
0000:4f:00.0 (144d a80a): uio_pci_generic -> nvme
0000:6d:00.0 (144d a80a): uio_pci_generic -> nvme
0000:cd:00.0 (144d a80a): uio_pci_generic -> nvme
0000:6e:00.0 (144d a80a): uio_pci_generic -> nvme
0000:6c:00.0 (144d a80a): uio_pci_generic -> nvme
0000:80:01.3 (8086 0b00): uio_pci_generic -> ioatdma
0000:80:01.2 (8086 0b00): uio_pci_generic -> ioatdma
0000:d1:00.0 (144d a80a): uio_pci_generic -> nvme
0000:80:01.1 (8086 0b00): uio_pci_generic -> ioatdma
0000:d0:00.0 (144d a80a): uio_pci_generic -> nvme
0000:80:01.0 (8086 0b00): uio_pci_generic -> ioatdma
0000:80:01.7 (8086 0b00): uio_pci_generic -> ioatdma
0000:80:01.6 (8086 0b00): uio_pci_generic -> ioatdma
0000:80:01.5 (8086 0b00): uio_pci_generic -> ioatdma
0000:ec:00.0 (144d a80a): uio_pci_generic -> nvme
0000:80:01.4 (8086 0b00): uio_pci_generic -> ioatdma
0000:cc:00.0 (144d a80a): uio_pci_generic -> nvme
0000:6b:00.0 (144d a80a): uio_pci_generic -> nvme
0000:cf:00.0 (144d a80a): uio_pci_generic -> nvme
0000:e9:00.0 (144d a80a): uio_pci_generic -> nvme
0000:67:00.0 (144d a80a): uio_pci_generic -> nvme
0000:d2:00.0 (144d a80a): uio_pci_generic -> nvme
0000:eb:00.0 (144d a80a): uio_pci_generic -> nvme
0000:d3:00.0 (144d a80a): uio_pci_generic -> nvme
0000:51:00.0 (144d a80a): uio_pci_generic -> nvme
0000:69:00.0 (144d a80a): uio_pci_generic -> nvme
0000:e5:00.0 (144d a80a): uio_pci_generic -> nvme
0000:53:00.0 (144d a80a): uio_pci_generic -> nvme
0000:00:01.3 (8086 0b00): uio_pci_generic -> ioatdma
0000:00:01.2 (8086 0b00): uio_pci_generic -> ioatdma
0000:e6:00.0 (144d a80a): uio_pci_generic -> nvme
0000:00:01.1 (8086 0b00): uio_pci_generic -> ioatdma
0000:e7:00.0 (144d a80a): uio_pci_generic -> nvme
0000:00:01.0 (8086 0b00): uio_pci_generic -> ioatdma
0000:00:01.7 (8086 0b00): uio_pci_generic -> ioatdma
0000:00:01.6 (8086 0b00): uio_pci_generic -> ioatdma
0000:00:01.5 (8086 0b00): uio_pci_generic -> ioatdma
0000:00:01.4 (8086 0b00): uio_pci_generic -> ioatdma
0000:54:00.0 (144d a80a): uio_pci_generic -> nvme
0000:52:00.0 (144d a80a): uio_pci_generic -> nvme
0000:e8:00.0 (144d a80a): uio_pci_generic -> nvme
0000:6a:00.0 (144d a80a): uio_pci_generic -> nvme
Setting maximum # of Huge Page Size is 128GB
0000:4e:00.0 (144d a80a): no driver -> uio_pci_generic
0000:ce:00.0 (144d a80a): no driver -> uio_pci_generic
0000:ea:00.0 (144d a80a): no driver -> uio_pci_generic
0000:68:00.0 (144d a80a): no driver -> uio_pci_generic
0000:4d:00.0 (144d a80a): no driver -> uio_pci_generic
0000:50:00.0 (144d a80a): no driver -> uio_pci_generic
0000:4f:00.0 (144d a80a): no driver -> uio_pci_generic
0000:6d:00.0 (144d a80a): no driver -> uio_pci_generic
0000:cd:00.0 (144d a80a): no driver -> uio_pci_generic
0000:80:01.3 (8086 0b00): ioatdma -> uio_pci_generic
0000:6e:00.0 (144d a80a): no driver -> uio_pci_generic
0000:6c:00.0 (144d a80a): no driver -> uio_pci_generic
0000:80:01.2 (8086 0b00): ioatdma -> uio_pci_generic
0000:80:01.1 (8086 0b00): ioatdma -> uio_pci_generic
0000:d1:00.0 (144d a80a): no driver -> uio_pci_generic
0000:80:01.0 (8086 0b00): ioatdma -> uio_pci_generic
0000:d0:00.0 (144d a80a): no driver -> uio_pci_generic
0000:80:01.7 (8086 0b00): ioatdma -> uio_pci_generic
0000:80:01.6 (8086 0b00): ioatdma -> uio_pci_generic
0000:80:01.5 (8086 0b00): ioatdma -> uio_pci_generic
0000:80:01.4 (8086 0b00): ioatdma -> uio_pci_generic
0000:ec:00.0 (144d a80a): no driver -> uio_pci_generic
0000:cc:00.0 (144d a80a): no driver -> uio_pci_generic
0000:6b:00.0 (144d a80a): no driver -> uio_pci_generic
0000:cf:00.0 (144d a80a): no driver -> uio_pci_generic
0000:e9:00.0 (144d a80a): no driver -> uio_pci_generic
0000:67:00.0 (144d a80a): no driver -> uio_pci_generic
0000:d2:00.0 (144d a80a): no driver -> uio_pci_generic
0000:eb:00.0 (144d a80a): no driver -> uio_pci_generic
0000:d3:00.0 (144d a80a): no driver -> uio_pci_generic
0000:51:00.0 (144d a80a): no driver -> uio_pci_generic
0000:69:00.0 (144d a80a): no driver -> uio_pci_generic
0000:00:01.3 (8086 0b00): ioatdma -> uio_pci_generic
0000:e5:00.0 (144d a80a): no driver -> uio_pci_generic
0000:53:00.0 (144d a80a): no driver -> uio_pci_generic
0000:00:01.2 (8086 0b00): ioatdma -> uio_pci_generic
0000:00:01.1 (8086 0b00): ioatdma -> uio_pci_generic
0000:e6:00.0 (144d a80a): no driver -> uio_pci_generic
0000:00:01.0 (8086 0b00): ioatdma -> uio_pci_generic
0000:e7:00.0 (144d a80a): no driver -> uio_pci_generic
0000:00:01.7 (8086 0b00): ioatdma -> uio_pci_generic
0000:00:01.6 (8086 0b00): ioatdma -> uio_pci_generic
0000:00:01.5 (8086 0b00): ioatdma -> uio_pci_generic
0000:00:01.4 (8086 0b00): ioatdma -> uio_pci_generic
0000:54:00.0 (144d a80a): no driver -> uio_pci_generic
0000:52:00.0 (144d a80a): no driver -> uio_pci_generic
0000:e8:00.0 (144d a80a): no driver -> uio_pci_generic
0000:6a:00.0 (144d a80a): no driver -> uio_pci_generic
/root/doc_center/ibofos/script
apport.service is not a native service, redirecting to systemd-sysv-install.
Executing: /lib/systemd/systemd-sysv-install disable apport
Current maximum # of memory map areas per process is 65535.
Setup env. done!
Execute poseidonos
Wait poseidonos
Wait poseidonos
Wait poseidonos
poseidonos is running in background...logfile=pos.log

# Verify if the application is up and running
root@R2U14-PSD-3:/poseidonos/script# ps -ef | grep poseidonos | grep -v grep
root     90998     1 99 20:09 pts/7    01:15:34 /root/doc_center/ibofos/script/..//bin/poseidonos

# Unlike in the the previous execution, you shouldn't see the NVMe devices from the fdisk output since all of them must have been reattached from OS to SPDK.
root@R2U14-PSD-3:/poseidonos/script# fdisk -l |grep nvme
```

### Step 2. Create Write Buffer within DRAM

```bash
root@R2U14-PSD-3:/poseidonos/script# cd ../lib/spdk-20.10/scripts/
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts#
 
# Check the usage to create write buffer for POS array
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# ./rpc.py bdev_malloc_create -h
usage: rpc.py [options] bdev_malloc_create [-h] [-b NAME] [-u UUID]
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
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# ./rpc.py bdev_malloc_create -b uram0 8192 512
uram0
```

```bash
The recommended size of uram0 may differ by environment. Please refer to "bdev" section in Learning POS Environment for further details.
```

### Step 3. Check POS information and version

```bash
# The actual output may differ by env where the command is executed.
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# cd /poseidonos/bin/
root@R2U14-PSD-3:/poseidonos/bin# ./cli system info
 
 
Request to Poseidon OS
    xrId        :  7dac37be-d93e-11eb-b677-a0369f78dee4
    command     :  GETIBOFOSINFO
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "version": "pos-0.9.2"
}
```

### Step 4. Scan NVMe Devices

```bash
# If this is the first run, you wouldn't see any devices showing up in the output as in the following.
root@R2U14-PSD-3:/poseidonos/bin# ./cli device list
 
 
Request to Poseidon OS
    xrId        :  fa29b0b0-d93e-11eb-8fce-a0369f78dee4
    command     :  LISTDEVICE
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
 
# Let's run the scan so that POS can detect devices.
root@R2U14-PSD-3:/poseidonos/bin# ./cli device scan
 
 
Request to Poseidon OS
    xrId        :  fc44f1aa-d93e-11eb-9e3d-a0369f78dee4
    command     :  SCANDEVICE
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
 
# This time, we should see a full list of devices
root@R2U14-PSD-3:/poseidonos/bin# ./cli device list
 
 
Request to Poseidon OS
    xrId        :  fe4b9b38-d93e-11eb-ae7a-a0369f78dee4
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
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045104      ",
            "type": "SSD"
        },
        {
            "addr": "0000:4e:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-1",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045041      ",
            "type": "SSD"
        },
        {
            "addr": "0000:4f:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-2",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045050      ",
            "type": "SSD"
        },
        {
            "addr": "0000:50:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-3",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045038      ",
            "type": "SSD"
        },
        {
            "addr": "0000:51:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-4",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045107      ",
            "type": "SSD"
        },
        {
            "addr": "0000:52:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-5",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045042      ",
            "type": "SSD"
        },
        {
            "addr": "0000:53:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-6",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045040      ",
            "type": "SSD"
        },
        {
            "addr": "0000:54:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-7",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045083      ",
            "type": "SSD"
        },
        {
            "addr": "0000:67:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-8",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045049      ",
            "type": "SSD"
        },
        {
            "addr": "0000:68:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-9",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045096      ",
            "type": "SSD"
        },
        {
            "addr": "0000:69:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-10",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045032      ",
            "type": "SSD"
        },
        {
            "addr": "0000:6a:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-11",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045071      ",
            "type": "SSD"
        },
        {
            "addr": "0000:6b:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-12",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045099      ",
            "type": "SSD"
        },
        {
            "addr": "0000:6c:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-13",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045098      ",
            "type": "SSD"
        },
        {
            "addr": "0000:6d:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-14",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045084      ",
            "type": "SSD"
        },
        {
            "addr": "0000:6e:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-15",
            "numa": "0",
            "size": 3840755982336,
            "sn": "A000032M045105      ",
            "type": "SSD"
        },
        {
            "addr": "0000:cc:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-16",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045220      ",
            "type": "SSD"
        },
        {
            "addr": "0000:cd:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-17",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045088      ",
            "type": "SSD"
        },
        {
            "addr": "0000:ce:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-18",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045090      ",
            "type": "SSD"
        },
        {
            "addr": "0000:cf:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-19",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045078      ",
            "type": "SSD"
        },
        {
            "addr": "0000:d0:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-20",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045077      ",
            "type": "SSD"
        },
        {
            "addr": "0000:d1:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-21",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045051      ",
            "type": "SSD"
        },
        {
            "addr": "0000:d2:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-22",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045034      ",
            "type": "SSD"
        },
        {
            "addr": "0000:d3:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-23",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045085      ",
            "type": "SSD"
        },
        {
            "addr": "0000:e5:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-24",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045053      ",
            "type": "SSD"
        },
        {
            "addr": "0000:e6:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-25",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045054      ",
            "type": "SSD"
        },
        {
            "addr": "0000:e7:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-26",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045048      ",
            "type": "SSD"
        },
        {
            "addr": "0000:e8:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-27",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045039      ",
            "type": "SSD"
        },
        {
            "addr": "0000:e9:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-28",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045087      ",
            "type": "SSD"
        },
        {
            "addr": "0000:ea:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-29",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045086      ",
            "type": "SSD"
        },
        {
            "addr": "0000:eb:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-30",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045100      ",
            "type": "SSD"
        },
        {
            "addr": "0000:ec:00.0",
            "class": "ARRAY",
            "mn": "SAMSUNG NVMe SSD PM9A3                  ",
            "name": "unvme-ns-31",
            "numa": "1",
            "size": 3840755982336,
            "sn": "A000032M045055      ",
            "type": "SSD"
        },
        {
            "addr": "",
            "class": "ARRAY",
            "mn": "uram0",
            "name": "uram0",
            "numa": "UNKNOWN",
            "size": 8589934592,
            "sn": "uram0",
            "type": "NVRAM"
        }
    ]
}
```

### Step 5. Import POS Array

If you have not created any POS array before, you could start with a new one and import into the POS (Step 5a). Otherwise, you could skip this step. 

### 5a. Create new POS Array

Now that POS has completed the scanning, it should be able to create POS array with a set of block devices we choose.

```bash
root@R2U14-PSD-3:/poseidonos/bin#  ./cli array create -b uram0 -d unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3,unvme-ns-4,unvme-ns-5,unvme-ns-6,unvme-ns-7,unvme-ns-8,unvme-ns-9,unvme-ns-10,unvme-ns-11,unvme-ns-12,unvme-ns-13,unvme-ns-14,unvme-ns-15,unvme-ns-16,unvme-ns-17,unvme-ns-18,unvme-ns-19,unvme-ns-20,unvme-ns-21,unvme-ns-22,unvme-ns-23,unvme-ns-24,unvme-ns-25,unvme-ns-26,unvme-ns-27,unvme-ns-28 -s unvme-ns-29,unvme-ns-30,unvme-ns-31 --name POSArray --raidtype RAID5
 
 
Request to Poseidon OS
    xrId        :  5512d085-d940-11eb-a518-a0369f78dee4
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
 - The available RAID types (--raidtype) are only ["RAID5"]

Once POS array has been created, you could query the POS array information as in the following:

```bash
root@R2U14-PSD-3:/poseidonos/bin# ./cli array list_device --name POSArray
 
 
Request to Poseidon OS
    xrId        :  8d74f019-d940-11eb-a553-a0369f78dee4
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

This step is not needed any more since POS version 0.9.2. In earlier version, "load" command was explicitly used to read MBR information and put array in unmounted state. This procedure has become part of "scan" command, where POS will automatically read MBR and load the array during "scan" command execution. 

### Step 6. Mount POS Array 
Even though we have POS array provisioned, we can't use it until it is mounted. Let's check out what happens with system state around POS array mount.

```bash
# Check if the array information is valid
root@R2U14-PSD-3:/poseidonos/bin# ./cli array info --name POSArray
 
 
Request to Poseidon OS
    xrId        :  6f7060b8-d943-11eb-978d-a0369f78dee4
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
    "capacity": "0GB (0B)",
    "createDatetime": "2021-06-30 10:35:00 +0900",
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
    ],
    "name": "POSArray",
    "rebuildingProgress": "0",
    "situation": "DEFAULT",
    "state": "OFFLINE",
    "updateDatetime": "2021-06-30 10:35:00 +0900",
    "used": "0GB (0B)"
}
 
# Let's mount the array
root@R2U14-PSD-3:/poseidonos/bin# ./cli array mount --name POSArray
 
 
Request to Poseidon OS
    xrId        :  8d9303e4-d943-11eb-88b5-a0369f78dee4
    command     :  MOUNTARRAY
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
 
 
# Check if the array state has become "NORMAL"
root@R2U14-PSD-3:/poseidonos/bin# ./cli array info --name POSArray
 
 
Request to Poseidon OS
    xrId        :  adbfa8e0-d943-11eb-93e3-a0369f78dee4
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
    "capacity": "94.832555773133TB (94832555773133B)",
    "createDatetime": "2021-06-30 10:35:00 +0900",
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
    ],
    "name": "POSArray",
    "rebuildingProgress": "0",
    "situation": "NORMAL",
    "state": "NORMAL",
    "updateDatetime": "2021-06-30 10:35:00 +0900",
    "used": "0GB (0B)"
}
```

Please note that state field in the output has changed from OFFLINE to NORMAL. Also, "capacity" is now reflecting the size of the NVMe storage pool available to POS. 

### Step 7. Configure NVM Subsystems for NVMe Over Fabric Target
POS is ready to perform volume management task, but still unable to expose its volume over network since we haven't configured an NVM subsystem yet. POS is not ready to expose its volume over network since it does not have NVM subsystem in which NVM namespaces(s) are created. Creating NVM subsystem remains in manual fashion  (vs. running automatically during POS startup) by design. Administrators need to understand its functionality so that they can easily come up with a workaround when needed. Once we have enough understanding about various user environments, this step could be automated in a future release.

#### Create NVMe-oF Subsystem
```bash
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# cd /poseidonos/bin/
root@R2U14-PSD-3:/poseidonos/bin# cd /poseidonos/lib/spdk-20.10/scripts/
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# ./rpc.py nvmf_create_subsystem -h
usage: rpc.py [options] nvmf_create_subsystem [-h] [-t TGT_NAME]
                                              [-s SERIAL_NUMBER]
                                              [-d MODEL_NUMBER] [-a]
                                              [-m MAX_NAMESPACES] [-r]
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
  -r, --ana-reporting   Enable ANA reporting feature
  
 
# If successful, the following doesn't print out any response
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# ./rpc.py nvmf_create_subsystem nqn.2019-04.ibof:subsystem1 -a -s IBOF00000000000001 -d IBOF_VOLUME_EXTENSION -m 256
```

The following command configures TCP transport to use when network connection is established between an initiator and a target. is between initiator and target. 

#### Create NVMe-oF Transport
```bash
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# ./rpc.py nvmf_create_transport -h
usage: rpc.py [options] nvmf_create_transport [-h] -t TRTYPE [-g TGT_NAME]
                                              [-q MAX_QUEUE_DEPTH]
                                              [-p MAX_QPAIRS_PER_CTRLR]
                                              [-m MAX_IO_QPAIRS_PER_CTRLR]
                                              [-c IN_CAPSULE_DATA_SIZE]
                                              [-i MAX_IO_SIZE]
                                              [-u IO_UNIT_SIZE]
                                              [-a MAX_AQ_DEPTH]
                                              [-n NUM_SHARED_BUFFERS]
                                              [-b BUF_CACHE_SIZE]
                                              [-s MAX_SRQ_DEPTH] [-r] [-o]
                                              [-f] [-y SOCK_PRIORITY]
                                              [-l ACCEPTOR_BACKLOG]
                                              [-x ABORT_TIMEOUT_SEC] [-w]
 
optional arguments:
  -h, --help            show this help message and exit
  -t TRTYPE, --trtype TRTYPE
                        Transport type (ex. RDMA)
  -g TGT_NAME, --tgt_name TGT_NAME
                        The name of the parent NVMe-oF target (optional)
  -q MAX_QUEUE_DEPTH, --max-queue-depth MAX_QUEUE_DEPTH
                        Max number of outstanding I/O per queue
  -p MAX_QPAIRS_PER_CTRLR, --max-qpairs-per-ctrlr MAX_QPAIRS_PER_CTRLR
                        Max number of SQ and CQ per controller. Deprecated,
                        use max-io-qpairs-per-ctrlr
  -m MAX_IO_QPAIRS_PER_CTRLR, --max-io-qpairs-per-ctrlr MAX_IO_QPAIRS_PER_CTRLR
                        Max number of IO qpairs per controller
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
  -l ACCEPTOR_BACKLOG, --acceptor_backlog ACCEPTOR_BACKLOG
                        Pending connections allowed at one time. Relevant only
                        for RDMA transport
  -x ABORT_TIMEOUT_SEC, --abort-timeout-sec ABORT_TIMEOUT_SEC
                        Abort execution timeout value, in seconds
  -w, --no-wr-batching  Disable work requests batching. Relevant only for RDMA
                        transport
 
 
# If successful, the following doesn't print out any response
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# ./rpc.py nvmf_create_transport -t tcp -b 64 -n 4096
```

The following command makes a given NVM subsystem listen on a TCP port and serve incoming NVMe-oF requests. 

#### Add NVMe-oF Subsystem Listener
```bash
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# ./rpc.py nvmf_subsystem_add_listener -h
usage: rpc.py [options] nvmf_subsystem_add_listener [-h] -t TRTYPE -a TRADDR
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
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# ifconfig
enp0s20f0u8u3c2: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        ether c6:a7:37:55:cd:ff  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 9714  bytes 1626100 (1.6 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
 
ens17f0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 9000
        inet 10.100.2.14  netmask 255.255.255.0  broadcast 10.100.2.255
        inet6 fe80::63f:72ff:febf:38de  prefixlen 64  scopeid 0x20<link>
        ether 04:3f:72:bf:38:de  txqueuelen 1000  (Ethernet)
        RX packets 15380  bytes 4253283 (4.2 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 524  bytes 46571 (46.5 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
 
ens17f1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 9000
        inet 10.100.3.14  netmask 255.255.255.0  broadcast 10.100.3.255
        inet6 fe80::63f:72ff:febf:38df  prefixlen 64  scopeid 0x20<link>
        ether 04:3f:72:bf:38:df  txqueuelen 1000  (Ethernet)
        RX packets 15382  bytes 4253753 (4.2 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 522  bytes 46494 (46.4 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
 
ens21f0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 10.1.2.14  netmask 255.255.0.0  broadcast 10.1.255.255
        inet6 fe80::a236:9fff:fe78:dee4  prefixlen 64  scopeid 0x20<link>
        ether a0:36:9f:78:de:e4  txqueuelen 1000  (Ethernet)
        RX packets 282894  bytes 29772255 (29.7 MB)
        RX errors 0  dropped 286  overruns 0  frame 0
        TX packets 9347  bytes 1226074 (1.2 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
 
ens21f1: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether a0:36:9f:78:de:e6  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
 
lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 30454  bytes 2290385 (2.2 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 30454  bytes 2290385 (2.2 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
 
# Pick up one of the NICs (e.g., ens17f0) and pass it to -a option. If successful, the following doesn't print out any response
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# ./rpc.py nvmf_subsystem_add_listener nqn.2019-04.ibof:subsystem1 -t tcp -a 10.100.2.14 -s 1158
```
In the above example, the NVM subsystem called "nqn.2019-04.ibof:subsystem1" has been configured to listen on (10.100.2.14, 1158) and use TCP transport. If you miss this step, POS wouldn't be able to mount POS volumes even though it could create new ones. 
At this point, you should be able to retrieve the configured NVM subsystem like in the following:

#### Retrieve NVM subsystem information
```bash
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# ./rpc.py nvmf_get_subsystems
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
        "traddr": "10.100.2.14",
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

#### Create a volume
```bash
# Create a 50-TB volume
root@R2U14-PSD-3:/poseidonos/bin# ./cli volume create --name vol1 --size 54975581388800 --maxiops 0 --maxbw 0 --array POSArray
 
 
Request to Poseidon OS
    xrId        :  aee4416b-dac1-11eb-b080-a0369f78dee4
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
 
 
# Check the volume information
root@R2U14-PSD-3:/poseidonos/bin# ./cli volume list --array POSArray
 
 
Request to Poseidon OS
    xrId        :  f7b3d6ec-dac1-11eb-b9f7-a0369f78dee4
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

Please note that the initial status of POS volume is Unmounted. 

### Step 9. Mount POS Volume

This is to make a particular POS volume ready to perform IO. After this step, POS volume is attached as bdev to an NVM subsystem and seen as an NVM namespace. 
```bash
# Mount the volume
root@R2U14-PSD-3:/poseidonos/bin# ./cli volume mount --name vol1 --array POSArray
 
 
Request to Poseidon OS
    xrId        :  32527ba5-dac2-11eb-870b-a0369f78dee4
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
 
 
# Check if the volume status is "Mounted" and the size is 50 TB
root@R2U14-PSD-3:/poseidonos/bin# ./cli volume list --array POSArray
 
 
Request to Poseidon OS
    xrId        :  4f8680aa-dac2-11eb-9f74-a0369f78dee4
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
```

Please note that the status of the volume has become Mounted.  If we check the NVM subsystem again, we can notice an NVM namespace has been added to an NVM subsystem with its bdev_name as follows.

#### Retrieve NVM subsystem information
```bash
# Check if nvmf_get_subsystems() command shows NVM namespace information that contains "bdev_0_POSArray"
root@R2U14-PSD-3:/poseidonos/bin# cd /poseidonos/lib/spdk-20.10/scripts/
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# ./rpc.py nvmf_get_subsystems
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
        "traddr": "10.100.2.14",
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
        "bdev_name": "bdev_0_POSArray",
        "name": "bdev_0_POSArray",
        "uuid": "c5705687-d266-4a1b-99b1-bb204ab0e3b8"
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
# Unmount the volume
root@R2U14-PSD-3:/poseidonos/lib/spdk-20.10/scripts# cd /poseidonos/bin/
root@R2U14-PSD-3:/poseidonos/bin# ./cli volume unmount --name vol1 --array POSArray
 
 
Request to Poseidon OS
    xrId        :  d90aa185-dac2-11eb-8e19-a0369f78dee4
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
 
 
# Check if the volume status is now "Unmounted"
root@R2U14-PSD-3:/poseidonos/bin# ./cli volume list --array POSArray
 
 
Request to Poseidon OS
    xrId        :  e0589732-dac2-11eb-97e7-a0369f78dee4
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
Please note that the status of the POS volume has changed from "Mounted" to "Unmounted".


### Step 11. Delete POS Volume
```bash
# Delete the volume
root@R2U14-PSD-3:/poseidonos/bin# ./cli volume delete --name vol1 --array POSArray
 
 
Request to Poseidon OS
    xrId        :  15274937-dac3-11eb-9c21-a0369f78dee4
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
 
 
# Make sure the volume list output does not contain "vol1" anymore
root@R2U14-PSD-3:/poseidonos/bin# ./cli volume list --array POSArray
 
 
Request to Poseidon OS
    xrId        :  6266b333-dac4-11eb-8fa7-a0369f78dee4
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
# Unmount the array
root@R2U14-PSD-3:/poseidonos/bin# ./cli array unmount --name POSArray
 
 
Request to Poseidon OS
    xrId        :  b66e9e2b-dac4-11eb-b50e-a0369f78dee4
    command     :  UNMOUNTARRAY
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
 
 
# Make sure the array is now in "OFFLINE" state
root@R2U14-PSD-3:/poseidonos/bin# ./cli array info --name POSArray
 
 
Request to Poseidon OS
    xrId        :  c4c60226-dac4-11eb-a661-a0369f78dee4
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
    "capacity": "0GB (0B)",
    "createDatetime": "2021-07-02 08:08:23 +0900",
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
    ],
    "name": "POSArray",
    "rebuildingProgress": "0",
    "situation": "DEFAULT",
    "state": "OFFLINE",
    "updateDatetime": "2021-07-02 08:08:23 +0900",
    "used": "0GB (0B)"
}
```

### Step 13. Delete POS Array
```bash
# Delete the array. It make take a few minutes to fininsh. In this demonstration, it took 6 minutes.
root@R2U14-PSD-3:/poseidonos/bin# ./cli array delete --name POSArray
 
 
Request to Poseidon OS
    xrId        :  ff26cfef-dac4-11eb-afce-a0369f78dee4
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
 
 
# Make sure that POSArray does not show up in array list command
root@R2U14-PSD-3:/poseidonos/bin# ./cli array list
 
 
Request to Poseidon OS
    xrId        :  0210a23c-dac5-11eb-bc89-a0369f78dee4
    command     :  LISTARRAY
    Param       :
{}
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "arrayList": "There is no array"
}
```
POS array can be deleted only when it is in OFFLINE state.

### Step 14. Shut down POS application
```bash
# Trigger a shutdown. The command does not block on the shutdown result.
root@R2U14-PSD-3:/poseidonos/bin# ./cli system exit
 
 
Request to Poseidon OS
    xrId        :  a42c5e59-dac5-11eb-9b94-a0369f78dee4
    command     :  EXITIBOFOS
 
 
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
 
# Check if the poseidonos process is not shown from ps output. The shutdown process may take a few minutes to finish.
root@R2U14-PSD-3:/poseidonos/bin# ps -ef | grep poseidon | grep -v grep
```

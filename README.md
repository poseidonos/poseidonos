# Poseidon OS

PoseidonOS (POS) is a lightweight storage OS that offers the best performance and valuable features over a storage network. POS exploits the benefit of NVMe Solid State Drives (SSDs) by optimizing the storage stack and leveraging the state-of-the-art high-speed interface. Please find project details on the documentation page.


# Table of Contents
- [Download the Source Code](#download-the-source-code)
- [Install Prerequisites](#install-prerequisites)
- [Build POS](#build-pos)
- [Run POS](#run-pos)
- [Learn POS Commands](#learn-pos-commands)

## Download the Source Code

```bash
git clone https://github.com/poseidonos/poseidonos.git
```

## Install Prerequisites

pkgdep.sh will automatically install the required packages to build POS.

```bash
cd script
sudo ./pkgdep.sh
```

## Build POS

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

## Learn POS with Command Line Interface

Let's explore the features and the capabilities of POS. Using the POS command-line interface, you will learn to manage storage resources (e.g., devices, arrays, and volumes). The prerequisite knowledge is the minimum skill in Linux administration.

**Important Note**: we also provide [**PoseidonOS-GUI**](https://github.com/poseidonos/poseidonos-gui), a web-based graphical user interface (GUI).

### Environments
In this example, the following hardware and configurations are used:
```bash
Hardware: Poseidon server
 - Reference server hardware implementation engineered by Samsung and Inspur
 - The number of processors: 2
 - The number of memory slots: 32
 - Memory speed: 3200 MT/s
 - Network speed: up to 600 GbE
 - PCIe generation: gen4
 - Storage: E1.S SSD * 32 ea
```

| Config |  Value  |
| ------ | ------- |
| OS  | **Ubuntu 18.04 <br/> (Note: PoseidonOS is not officially supported in the later versions of Ubuntu.)** |
| Kernel     | 5.3.0-24-generic      |
| Hostname | R2U14-PSD-3 |
| $POS_HOME | /poseidonos |
| POS location | $POS_HOME/bin/poseidonos <br/> $POS_HOME/bin/poseidonos-cli |
| POS config | /etc/pos/pos.conf |
| POS scripts |  $POS_HOME/script/start_poseidonos.sh <br/> $POS_HOME/lib/spdk-20.10/script/rpc.py <br/> $POS_HOME/test/script/set_irq_affinity_cpulist.sh <br/> $POS_HOME/test/script/common_irq_affinity.sh <br/> $POS_HOME/script/setup_env.sh <br/> $POS_HOME/lib/spdk-20.10/scripts/setup.sh <br/> $POS_HOME/lib/spdk-20.10/scripts/common.sh |
| POS log | /var/log/pos/pos.log |
| POS dump | /etc/pos/core/poseidonos.core |
| SPDK RPC Server UDS | /var/tmp/spdk.sock |
| Hugepage information | /tmp/uram_hugepage |

### Step 1. Start POS application

Using the following commands (with the root permission), check out the NVMe devices attached to the OS. They are currently using the kernel device driver.
```bash
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
```

Start POS using the following command. The NVMe devices will be detached from the Linux kernel and attached to SPDK (a user-level device driver).
```bash
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
poseidonos is running in background...
root@R2U14-PSD-3:/poseidonos/script#
```

```bash
# Verify if the application is up and running
root@R2U14-PSD-3:/poseidonos/script# ps -ef | grep poseidonos | grep -v grep
root     90998     1 99 20:09 pts/7    01:15:34 /root/doc_center/ibofos/script/..//bin/poseidonos

# Unlike in the the previous execution, you shouldn't see the NVMe devices from the fdisk output since all of them must have been reattached from OS to SPDK.
root@R2U14-PSD-3:/poseidonos/script# fdisk -l | grep nvme
```

### Step 2. Create Write Buffer within DRAM
Using the device create command, create a write buffer in DRAM. A POS array will use it.
 
The following command will create *uram0*, a write buffer with a total size of 4096MB and a block size of 512B. The command will request an SPDK server to create an SPDK block device called *malloc bdev*, a userspace ramdisk.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli device create --device-name uram0 --device-type uram --num-blocks 8388608 --block-size 512
```

- Note 1: the recommended size of a write buffer is different by environment.
- Note 2: The size of a write buffer should be carefully chosen. For example, if the size of a write buffer is greater than 4096MB, it may not be possible to perform journaling-based sudden power-off recovery (SPOR).


### Step 3. Check POS version
You can check the version of POS using the system info command. 
```bash
# The actual output may differ by env where the command is executed.
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli system info
pos-0.9.10
```

### Step 4. Scan NVMe Devices
Let's check the list of the devices in the system. Because you haven't scanned the device in the system yet, you won't be able to see any device in the output.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli device list
Name           |SerialNumber(SN)    |Address        |Class         |MN                         |NUMA   |Size
-------------- |------------------- |-------------- |------------- |-------------------------- |------ |------------------
```
 
Let's scan devices using the device scan command.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli device scan
```
 
Now you can see the list of NVMe and uram devices in the system. 
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli device list --unit
Name           |SerialNumber(SN)     |Address        |Class         |MN                                       |NUMA    |Size
-------------- |-------------------  |-------------- |------------- |--------------------------               |------  |------------------
unvme-ns-0     |A000032M045220       |0000:4d:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-1     |A000032M045032       |0000:4e:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-2     |A000032M045090       |0000:4f:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-3     |A000032M045096       |0000:50:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-4     |A000032M045107       |0000:51:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-5     |A000032M045078       |0000:52:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-6     |A000032M045077       |0000:53:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-7     |A000032M045099       |0000:54:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-8     |A000032M045084       |0000:67:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-9     |A000032M045105       |0000:68:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-10    |A000032M045100       |0000:69:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-11    |A000032M045104       |0000:6a:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-12    |A000032M045050       |0000:6b:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-13    |A000032M045054       |0000:6c:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-14    |A000032M045087       |0000:6d:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-15    |A000032M045088       |0000:6e:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |0       |3.5T
unvme-ns-16    |A000032M045038       |0000:cc:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-17    |A000032M045086       |0000:cd:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-18    |A000032M045048       |0000:ce:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-19    |A000032M045049       |0000:cf:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-20    |A000032M045039       |0000:d0:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-21    |A000032M045034       |0000:d1:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-22    |A000032M045042       |0000:d2:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-23    |A000032M045085       |0000:d3:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-24    |A000032M045041       |0000:e5:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-25    |A000032M045051       |0000:e6:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-26    |A000032M045040       |0000:e7:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-27    |A000032M045071       |0000:e8:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-28    |A000032M045055       |0000:e9:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-29    |A000032M045083       |0000:ea:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-30    |A000032M045098       |0000:eb:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
unvme-ns-31    |A000032M045053       |0000:ec:00.0   |SYSTEM        |SAMSUNG NVMe SSD PM9A3                   |1       |3.5T
uram0          |uram0                |               |SYSTEM        |uram0                                    |UNKNOWN |8G
```

### Step 5. Create POS Array

We will explore creating a POS array, a storage pool in POS.

Create a POS array using the array create command. The array create command requires the following parameters:
 - The name of a device to be used as the write buffer (--buffer).
 - The comma-separated list of the data (--data-devs) and spare (--spare) devices. Only SSD devices must be used.
 - The name of POS array must follow the naming convention rule. It is described in **[Device and Array](doc/concepts/device_and_array.md)** in detail. 
 - The RAID type for the POS array (--raid).

```bash
root@R2U14-PSD-3:/poseidonos/bin#./poseidonos-cli array create --array-name POSArray --buffer uram0 --data-devs unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3,unvme-ns-4,unvme-ns-5,unvme-ns-6,unvme-ns-7,unvme-ns-8,unvme-ns-9,unvme-ns-10,unvme-ns-11,unvme-ns-12,unvme-ns-13,unvme-ns-14,unvme-ns-15,unvme-ns-16,unvme-ns-17,unvme-ns-18,unvme-ns-19,unvme-ns-20,unvme-ns-21,unvme-ns-22,unvme-ns-23,unvme-ns-24,unvme-ns-25,unvme-ns-26,unvme-ns-27,unvme-ns-28 --spare unvme-ns-29,unvme-ns-30,unvme-ns-31 --raid RAID5
```


Once a POS array has been created, you can list the POS array using the array list command as follows.

```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli array list
Index |Name       |DatetimeCreated           |DatetimeUpdated           |Status
----- |---------- |---------------------     |---------------------     |----------
0     |POSArray   |2021-09-10 16:25:04 +0900 |2021-09-10 16:25:04 +0900 |Unmounted
```

If you want to see more detailed information about the array, specify the array to the command as follows:

```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli array list --array-name POSArray --unit
Array : POSArray
------------------------------------
Index               : 0
State               : OFFLINE
Situation           : DEFAULT
Rebuilding Progress : 0
Total               : 86.2T
Used                : 0B
GCMode              :

Devices
Name        Type
----        ------
uram0       BUFFER
unvme-ns-0  DATA
unvme-ns-1  DATA
unvme-ns-2  DATA
unvme-ns-3  DATA
unvme-ns-4  DATA
unvme-ns-5  DATA
unvme-ns-6  DATA
unvme-ns-7  DATA
unvme-ns-8  DATA
unvme-ns-9  DATA
unvme-ns-10 DATA
unvme-ns-11 DATA
unvme-ns-12 DATA
unvme-ns-13 DATA
unvme-ns-14 DATA
unvme-ns-15 DATA
unvme-ns-16 DATA
unvme-ns-17 DATA
unvme-ns-18 DATA
unvme-ns-19 DATA
unvme-ns-20 DATA
unvme-ns-21 DATA
unvme-ns-22 DATA
unvme-ns-23 DATA
unvme-ns-24 DATA
unvme-ns-25 DATA
unvme-ns-26 DATA
unvme-ns-27 DATA
unvme-ns-28 DATA
unvme-ns-29 SPARE
unvme-ns-30 SPARE
unvme-ns-31 SPARE
```


### Step 6. Mount POS Array 
You can create a POS volume from the POS array created in the previous step. However, you need to mount the array first. Let's check out the status of the POS array.

The array list command will display the status of POS arrays. You can see POSArray is being **Unmounted**.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli array list
Index |Name       |DatetimeCreated           |DatetimeUpdated           |Status
----- |---------- |---------------------     |---------------------     |----------
0     |POSArray   |2021-09-10 16:25:04 +0900 |2021-09-10 16:25:04 +0900 |Unmounted
```
 
Mount POSArray using the array mount command. It may take some time.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli array mount --array-name POSArray
```
 
Check out the status of POSArray has changed to **Mounted**.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli array list
Index |Name       |DatetimeCreated           |DatetimeUpdated           |Status
----- |---------- |---------------------     |---------------------     |----------
0     |POSArray   |2021-09-10 16:25:04 +0900 |2021-09-10 16:33:34 +0900 |Mounted
```

Let's execute the array list command with POSArray. Once POSArray is mounted, you can see that its state has changed from OFFLINE to NORMAL, which indicates that the array is ready to create POS volumes. 
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli array list --array-name POSArray --unit
Array : POSArray
------------------------------------
Index               : 0
State               : NORMAL
Situation           : NORMAL
Rebuilding Progress : 0
Total               : 86.2T
Used                : 0B
GCMode              : none

Devices
Name        Type
----        ------
uram0       BUFFER
unvme-ns-0  DATA
unvme-ns-1  DATA
unvme-ns-2  DATA
unvme-ns-3  DATA
unvme-ns-4  DATA
unvme-ns-5  DATA
unvme-ns-6  DATA
unvme-ns-7  DATA
unvme-ns-8  DATA
unvme-ns-9  DATA
unvme-ns-10 DATA
unvme-ns-11 DATA
unvme-ns-12 DATA
unvme-ns-13 DATA
unvme-ns-14 DATA
unvme-ns-15 DATA
unvme-ns-16 DATA
unvme-ns-17 DATA
unvme-ns-18 DATA
unvme-ns-19 DATA
unvme-ns-20 DATA
unvme-ns-21 DATA
unvme-ns-22 DATA
unvme-ns-23 DATA
unvme-ns-24 DATA
unvme-ns-25 DATA
unvme-ns-26 DATA
unvme-ns-27 DATA
unvme-ns-28 DATA
unvme-ns-29 SPARE
unvme-ns-30 SPARE
unvme-ns-31 SPARE
```

### Step 7. Configure NVM Subsystems for NVMe Over Fabric Target
Now POSArray is ready to create a POS volume. Before creating a POS volume, we will create and configure an NVM subsystem first. The NVM subsystem will allow us to expose the POS volume to initiators over the network. 

#### Create NVMe-oF Subsystem
Create an NVMe-oF subsystem using the subsystem create command.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli subsystem create --subnqn nqn.2019-04.ibof:subsystem1 --serial-number IBOF00000000000001 --model-number IBOF_VOLUME_EXTENSION --max-namespaces 256 -o
```
#### Create NVMe-oF Transport
Create an NVMf transport using the subsystem create-transport command.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli subsystem create-transport --trtype tcp -c 64 --num-shared-buf 4096
```

#### Add NVMe-oF Subsystem Listener
The subsystem add-listener command binds an NVM subsystem to a socket address. This allows the NVM subsystem to listen on a TCP port to serve incoming NVMe-oF requests. 
 
Check the available network interfaces using *ifconfig*.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ifconfig
ens21f0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 10.1.2.14  netmask 255.255.0.0  broadcast 10.1.255.255
        inet6 fe80::a236:9fff:fe78:dee4  prefixlen 64  scopeid 0x20<link>
        ether a0:36:9f:78:de:e4  txqueuelen 1000  (Ethernet)
        RX packets 10922807  bytes 4337740510 (4.3 GB)
        RX errors 0  dropped 748811  overruns 0  frame 0
        TX packets 830274  bytes 71577491 (71.5 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 235840  bytes 16797914 (16.7 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 235840  bytes 16797914 (16.7 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```
 
Execute the add-listener command with the IP address and the port number of one of the available NICs (e.g., ens21f0).
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli subsystem add-listener -q nqn.2019-04.ibof:subsystem1 -t tcp -i 10.1.2.14 -p 1158
```
In the above example, the NVM subsystem "nqn.2019-04.ibof:subsystem1" has been configured to listen on socket address "10.1.2.14:1158" using TCP. If you omit this step, POS wouldn't be able to mount POS volumes. 

#### Retrieve NVM subsystem information
At this point, you should be able to retrieve the configured NVM subsystem as in the following:
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli subsystem list
Name                                  |Subtype     |AddressCount |SerialNumber(SN)      |ModelNumber(MN)       |NamespaceCount
------------------------------------- |----------- |------------ |--------------------- |--------------------- |--------------
nqn.2014-08.org.nvmexpress.discovery  |Discovery   |0            |                      |                      |0
nqn.2019-04.ibof:subsystem1           |NVMe        |1            |IBOF00000000000001    |IBOF_VOLUME_EXTENSION |0
```

### Step 8. Create POS Volume

In this step, we will create a POS volume, a logical entry point from the target side IO, which will be shown as a namespace in an NVM subsystem. A POS volume is wrapped as a ***bdev*** and can be attached to an NVM subsystem. ***bdev*** is a block device abstraction offered by the SPDK library. 

#### Create a volume

Let's create a 50TB volume using the following command:
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli volume create --volume-name vol1 --array-name POSArray --size 50TB --maxiops 0 --maxbw 0
```

Check the volume information using the volume list command.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli volume list --array-name POSArray --unit
Name      |ID    |TotalCapacity                |RemainingCapacity            |Remaining% |Status     |MaximumIOPS      |MaximumBandwith
--------- |----- |---------------------------- |---------------------------- |---------  |---------- |---------------- |----------------
vol1      |0     |50T                          |0B                           |0          |Unmounted  |0                |0
```
- Note 1: the initial status of a POS volume newly created is set to **Unmounted**.
- Note 2: you can see the remaining capacity is 0B. This is because the volume is unmounted.

### Step 9. Mount POS Volume

We need to mount a POS volume to perform IO operations on it. After mounted, the POS volume will be attached to an NVM subsystem as a block device (bdev). Also, it will be seen as an NVM namespace. 

Mount the volume using the volume mount command.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli volume mount --volume-name vol1 --array-name POSArray
```
 
Check if the status and the remaning capacity of the volume have changed to "Mounted" and 50 TB, respectively.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli volume list --array-name POSArray --unit
Name      |ID    |TotalCapacity                |RemainingCapacity            |Remaining% |Status     |MaximumIOPS      |MaximumBandwith
--------- |----- |---------------------------- |---------------------------- |---------  |---------- |---------------- |----------------
vol1      |0     |50T                          |50T                          |100        |Mounted    |0                |0
```
- Note: the RemainingCapacity of a newly created volume is set to the TotalCapacity when the volume is mounted for the first time. 
  Once every block of the volume is touched/written, the RemainingCapacity would remain at 0 until the volume gets deleted.
  The RemainingCapacity captures the internal state of block mappings and should not be interpreted as user's file system free space.

#### Retrieve NVM subsystem information
Using the subsystem list command, you can see the NVM namespace has been added to the NVM subsystem with its bdev_name.

Check if the NVM namespace information contains "bdev_0_POSArray" using the subsystem list command.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli subsystem list --subnqn nqn.2019-04.ibof:subsystem1
nqn              : nqn.2019-04.ibof:subsystem1
subtype          : NVMe
listen_addresses :
                   {
                     trtype : TCP
                     adrfam : IPv4
                     traddr : 10.1.2.14
                     trsvcid : 1158
                   }
allow_any_host   : true
hosts            :
serial_number    : IBOF00000000000001
model_number     : IBOF_VOLUME_EXTENSION
max_namespaces   : 256
namespaces       :
                   {
                     nsid : 1
                     bdev_name : bdev_0_POSArray
                     uuid : ddbacc0d-aec2-47e3-8654-4aa560fd549c
                   }
```

Once mounted, the connection is established between an initiator and an NVM subsystem. Then, POS volume becomes accessible over network by an initiator.


### Step 10. Unmount POS Volume
Unmount the volume.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli volume unmount --volume-name vol1 --array-name POSArray
WARNING: After unmounting volume vol1 in array POSArray, the progressing I/Os may fail if any.

Are you sure you want to unmount volume vol1? (y/n):y
```
 
 
Check if the volume status is now "Unmounted".
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli volume list --array-name POSArray --unit
Name      |ID    |TotalCapacity                |RemainingCapacity            |Remaining% |Status     |MaximumIOPS      |MaximumBandwith
--------- |----- |---------------------------- |---------------------------- |---------  |---------- |---------------- |----------------
vol1      |0     |50T                          |0B                           |0          |Unmounted  |0                |0
```


### Step 11. Delete POS Volume
Delete the volume.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli volume delete --volume-name vol1 --array-name POSArray
WARNING: After deleting volume vol1, you cannot recover the data of volume vol1 in the array POSArray

Are you sure you want to delete volume vol1? (y/n):y
```

 
Make sure that the volume list command does not display "vol1".
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli volume list --array-name POSArray --unit
Name      |ID    |TotalCapacity                |RemainingCapacity            |Remaining% |Status     |MaximumIOPS      |MaximumBandwith
--------- |----- |---------------------------- |---------------------------- |---------  |---------- |---------------- |----------------
```
POS volume can be deleted only when it is in the Unmounted state. 

### Step 12. Unmount POS Array
Unmount the array.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli array unmount --array-name POSArray
WARNING: After unmounting array POSArray, all the volumes in the array will be unmounted.
In addition, progressing I/Os may fail if any.

Are you sure you want to unmount array POSArray? (y/n):y
```
 
 
Make sure that the status of POSArray is now "Unmounted".
```bash
Index |Name       |DatetimeCreated           |DatetimeUpdated           |Status
----- |---------- |---------------------     |---------------------     |----------
0     |POSArray   |2021-09-10 16:25:04 +0900 |2021-09-10 16:33:34 +0900 |Unmounted
```

### Step 13. Delete POS Array
Delete the array. It make take a few minutes to fininsh. In this demonstration, it took 6 minutes.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli array delete --array-name POSArray
WARNING: After deleting array POSArray, you cannot recover the data of the volumes in the array.

Are you sure you want to delete array POSArray? (y/n):y
```
 
 
Make sure that the array list command does not display POSArray.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli array list
Index |Name       |DatetimeCreated       |DatetimeUpdated       |Status
----- |---------- |--------------------- |--------------------- |----------
```
POS array can be deleted only when it is in the OFFLINE state.

### Step 14. Stop POS
Shutdown pos using the system stop commnad. The shutdown process may take a few minutes.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ./poseidonos-cli system stop
WARNING: Stopping POS will affect the progressing I/Os.

Are you sure you want to stop POS? (y/n):y
```

 
Check if the process of POS has been terminated using the command below.
```bash
root@R2U14-PSD-3:/poseidonos/bin# ps -ef | grep poseidon | grep -v grep
```

Now you have learned the basic of POS. If you want to get deeper into POS, check out the documents: [link](https://github.com/poseidonos/poseidonos/tree/main/doc)

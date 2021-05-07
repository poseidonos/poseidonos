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
sudo ./bin/poseidonos
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
OS: Ubuntu 18.04 (kernel: 5.3.0-19-generic)
Application binary: poseidonos, cli
Application config: /etc/pos/conf/pos.conf
Application scripts to configure environments
Writable file system for RPC socket, application logs/dump, and hugepage info
```

### Step 1. Start POS application

```bash
# Check if you have local NVMe devices attached to the OS with its Kernel Device Driver.
ibof@ibof-target:~$ sudo fdisk -l | grep nvme
Disk /dev/nvme0n1: 64 GiB, 68719476736 bytes, 134217728 sectors
Disk /dev/nvme1n1: 64 GiB, 68719476736 bytes, 134217728 sectors
Disk /dev/nvme2n1: 64 GiB, 68719476736 bytes, 134217728 sectors
Disk /dev/nvme3n1: 64 GiB, 68719476736 bytes, 134217728 sectors
 
# Become root and run POS application as a daemon process.
ibof@ibof-target:~$ sudo -s
 
# Start POS
root@ibof-target:IBOF_HOME/script# ./start_poseidonos.sh
 
# The first four messages show that the local NVMe devices have been detached from the OS, which exploits its Kernel Device Driver. The last four messages show that they are now attached to SPDK, which exploits Userspace Device Driver.
0000:04:00.0 (15ad 07f0): uio_pci_generic -> nvme
0000:0c:00.0 (15ad 07f0): uio_pci_generic -> nvme
0000:13:00.0 (15ad 07f0): uio_pci_generic -> nvme
0000:1b:00.0 (15ad 07f0): uio_pci_generic -> nvme
0000:04:00.0 (15ad 07f0): nvme -> uio_pci_generic
0000:0c:00.0 (15ad 07f0): nvme -> uio_pci_generic
0000:13:00.0 (15ad 07f0): nvme -> uio_pci_generic
0000:1b:00.0 (15ad 07f0): nvme -> uio_pci_generic
 
# The hugepage reservation is done at this step, but no message is displayed.
# Hugepages are reserved for about 1/3 of total size of primary memory in the target system.
 
/home/ibof/projects/poseidonos-devel/script
apport.service is not a native service, redirecting to systemd-sysv-install.
Executing: /lib/systemd/systemd-sysv-install disable apport
Current maximum # of memory map areas per process is 65535.
Setup env. done!
 
 
POS is running in background...logfile=pos.log
 
# Verify if the application is up and running
root@ibof-target:IBOF_HOME/script# ps -ef | grep poseidonos | grep -v grep
root      1400     1 99 Nov 26 ?      10-12:03:09 ../bin/poseidonos
 
# At this point, you shouldn't see NVMe devices except for the OS device itself if it is also an NVMe device.
# All of them should have been detached from the OS and attached to SPDK library with Userspace Device Driver.
ibof@ibof-target:~$ sudo fdisk -l | grep nvme
```

### Step 2. Create Write Buffer within DRAM
```bash
root@ibof-target:IBOF_HOME/lib/spdk-20.10/scripts# ./rpc.py bdev_malloc_create -h
usage: rpc.py bdev_malloc_create [-h] [-b NAME] [-u UUID]
                                 total_size block_size
 
positional arguments:
  total_size            Size of malloc bdev in MB (float > 0)
  block_size            Block size for this bdev
 
optional arguments:
  -h, --help            show this help message and exit
  -b NAME, --name NAME  Name of the bdev
  -u UUID, --uuid UUID  UUID of the bdev
 
# This command will create a SPDK block device called "malloc bdev" that is a userspace ramdisk with the total size of 8192 MB, the block size of 512 B, and name as "uram0".
# Currently, only 512 bytes of block size is supported for the write buffer.
root@ibof-target:IBOF_HOME/lib/spdk-20.10/scripts# ./rpc.py bdev_malloc_create -b uram0 8192 512
uram0
```
```bash
The recommended size of uram0 may differ by environment. Please refer to "bdev" section in Learning POS Environment for further details.
```

### Step 3. Check POS version

```bash
# Root is not necessary in order to run "cli" commands.
# The actual output may differ from this according to the env you execute the command
ibof@ibof-target:IBOF_HOME/bin$ ./cli system info
  
  
Request to Poseidon OS
    xrId        :  2825009e-2f88-11eb-8607-005056adb61a
    command     :  GETIBOFOSINFO
  
  
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
	"version": "pos-0.8.0"
}
```

### Step 4. Scan NVMe Devices

```bash
# If this is the first run, you wouldn't see any devices showing up in the output as in the following.
ibof@ibof-target:IBOF_HOME/bin$ ./cli device list
  
  
Request to Poseidon OS
    xrId        :  2067c0c1-2f8a-11eb-a513-005056adb61a
    command     :  LISTDEVICE
  
  
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
 
  
# Let POS scan devices. POS should discover NVMe devices and remember their information (in memory)
ibof@ibof-target:IBOF_HOME/bin$ ./cli device scan
  
  
Request to Poseidon OS
    xrId        :  4ce8066f-2f8a-11eb-9d5b-005056adb61a
    command     :  SCANDEVICE
  
  
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
  
# Let's give it one more try with "list". Since the local NVMe devices have been scanned, POS should
# be able to give us the list.
ibof@ibof-target:IBOF_HOME/bin$ ./cli device list
  
  
Request to Poseidon OS
    xrId        :  53d159b7-2f8a-11eb-8579-005056adb61a
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
            "addr": "0000:04:00.0",
            "class": "SYSTEM",
            "mn": "VMware Virtual NVMe Disk",
            "name": "unvme-ns-0",
            "size": 68719476736,
            "sn": "VMWare NVME-0002",
            "type": "SSD"
        },
        {
            "addr": "0000:0c:00.0",
            "class": "SYSTEM",
            "mn": "VMware Virtual NVMe Disk",
            "name": "unvme-ns-1",
            "size": 68719476736,
            "sn": "VMWare NVME-0003",
            "type": "SSD"
        },
        {
            "addr": "0000:13:00.0",
            "class": "SYSTEM",
            "mn": "VMware Virtual NVMe Disk",
            "name": "unvme-ns-2",
            "size": 68719476736,
            "sn": "VMWare NVME-0000",
            "type": "SSD"
        },
        {
            "addr": "0000:1b:00.0",
            "class": "SYSTEM",
            "mn": "VMware Virtual NVMe Disk",
            "name": "unvme-ns-3",
            "size": 68719476736,
            "sn": "VMWare NVME-0001",
            "type": "SSD"
        },
        {
            "addr": "",
            "class": "SYSTEM",
            "mn": "uram0",
            "name": "uram0",
            "size": 1073741824,
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
ibof@ibof-target:IBOF_HOME/bin$ ./cli array create -b uram0 -d unvme-ns-0,unvme-ns-1,unvme-ns-2 -s unvme-ns-3  --name POSArray --raidtype RAID5
  
  
Request to Poseidon OS
    xrId        :  8b68a56b-2f8a-11eb-af95-005056adb61a
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
        }
    ],
    "spare": [
        {
            "deviceName": "unvme-ns-3"
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
ibof@ibof-target:IBOF_HOME/bin$ ./cli array list_device --name POSArray
  
  
Request to Poseidon OS
    xrId        :  f16396ea-2f8a-11eb-bd77-005056adb61a
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
            "type": "SPARE"
        }
    ]
}
```

### Step 6. Mount POS Array 
Even though we have POS array provisioned, we can't use it until it is mounted. Let's check out what happens with array state around POS array mount.

```bash
ibof@ibof-target:IBOF_HOME/bin$ ./cli array info --name POSArray
  
  
Request to Poseidon OS
    xrId        :  3f30be48-2f8c-11eb-9afa-005056adb61a
    command     :  ARRAYINFO
  
  
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "name":"POSArray", 
    "state":"OFFLINE",
    "situation":"DEFAULT",
    "rebuilding_progress":0,
    "capacity":0,
    "used":0,
    "devicelist":[
        {
            "type":"BUFFER",
            "name":"uram0"
        },
        {
            "type":"DATA",
            "name":"unvme-ns-0"
        },
        {
            "type":"DATA",
            "name":"unvme-ns-1"
        },
        {
            "type":"DATA",
            "name":"unvme-ns-2"
        },
        {
            "type":"SPARE",
            "name":"unvme-ns-3"
        }
    ]
}

ibof@ibof-target:IBOF_HOME/bin$ ./cli array mount --name POSArray
  
  
Request to Poseidon OS
    xrId        :  31dbc740-2f8c-11eb-ae95-005056adb61a
    command     :  MOUNTARRAY
  
  
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
  
  
ibof@ibof-target:IBOF_HOME/bin$ ./cli array info --name POSArray
  
  
Request to Poseidon OS
    xrId        :  3f30be48-2f8c-11eb-9afa-005056adb61a
    command     :  ARRAYINFO
  
  
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "name":"POSArray", 
    "state":"BUSY",
    "situation":"REBUILDING",
    "rebuilding_progress":10,
    "capacity":120795955200,
    "used":107374182400,
    "devicelist":[
        {
            "type":"BUFFER",
            "name":"uram0"
        },
        {
            "type":"DATA",
            "name":"unvme-ns-0"
        },
        {
            "type":"DATA",
            "name":"unvme-ns-1"
        },
        {
            "type":"DATA",
            "name":"unvme-ns-2"
        },
        {
            "type":"SPARE",
            "name":"unvme-ns-3"
        }
    ]
}
```
"capacity" is now reflecting the size of the NVMe storage pool of an array. 
```bash
Please note that, as of Nov/30/2020, POS supports a single POS array only, which is why "mount" command belongs to "system" (i.e., it's currently "cli system mount", but not "cli array mount --name POSArray"). Once POS gets a new feature to support multi array, the command will change accordingly. 
```
Please note that "system mount" and "system unmount" commands are not supported from May/7/2020. Instead, please use "array mount" and "array unmount" commands. Additionally, to get the information of an array, please use "array info" command instead of "system info" command.


### Step 7. Configure NVM Subsystems for NVMe Over Fabric Target
POS is ready to perform volume management task, but still unable to expose its volume over network since we haven't configured an NVM subsystem yet. POS is not ready to expose its volume over network since it does not have NVM subsystem in which NVM namespaces(s) are created. Creating NVM subsystem remains in manual fashion  (vs. running automatically during POS startup) by design. Administrators need to understand its functionality so that they can easily come up with a workaround when needed. Once we have enough understanding about various user environments, this step could be automated in a future release.

Create NVMe-oF Subsystem
```bash
root@ibof-target:IBOF_HOME/lib/spdk-20.10/scripts# ./rpc.py nvmf_create_subsystem -h
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
  
root@ibof-target:IBOF_HOME/lib/spdk-20.10/scripts# ./rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem1 -a -s POS00000000000001 -d POS_VOLUME_EXTENSION -m 256
```

The following command configures TCP transport to use when network connection is established between an initiator and a target. is between initiator and target. 

Create NVMe-oF Transport
```bash
root@ibof-target:IBOF_HOME/lib/spdk-20.10/scripts# ./rpc.py nvmf_create_transport -h
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
  
root@ibof-target:IBOF_HOME/lib/spdk-20.10/scripts# ./rpc.py nvmf_create_transport -t tcp -b 64 -n 2048
```

The following command makes a given NVM subsystem listen on a TCP port and serve incoming NVMe-oF requests. 

Add NVMe-oF Subsystem Listener
```bash
root@ibof-target:IBOF_HOME/lib/spdk-20.10/scripts# ./rpc.py nvmf_subsystem_add_listener -h
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
 
 
root@ibof-target:IBOF_HOME/lib/spdk-20.10/scripts# ifconfig
br-ffbb105da0b0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.18.0.1  netmask 255.255.0.0  broadcast 172.18.255.255
        ether 02:42:bc:6d:42:c0  txqueuelen 0  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
  
docker0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.17.0.1  netmask 255.255.0.0  broadcast 172.17.255.255
        ether 02:42:5a:48:ef:31  txqueuelen 0  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
  
ens160: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 10.1.11.20  netmask 255.255.0.0  broadcast 10.1.255.255
        inet6 fe80::250:56ff:fead:b61a  prefixlen 64  scopeid 0x20<link>
        ether 00:50:56:ad:b6:1a  txqueuelen 1000  (Ethernet)
        RX packets 10347809  bytes 5642325066 (5.6 GB)
        RX errors 0  dropped 1807  overruns 0  frame 0
        TX packets 2678399  bytes 1342733373 (1.3 GB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
  
ens192f0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 9000
        inet 10.100.11.20  netmask 255.255.0.0  broadcast 10.100.255.255
        inet6 fe80::250:56ff:fead:62e3  prefixlen 64  scopeid 0x20<link>
        ether 00:50:56:ad:62:e3  txqueuelen 1000  (Ethernet)
        RX packets 49543  bytes 9172204 (9.1 MB)
        RX errors 0  dropped 522  overruns 0  frame 0
        TX packets 7280  bytes 636436 (636.4 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
  
lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 35058288  bytes 21787309364 (21.7 GB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 35058288  bytes 21787309364 (21.7 GB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
  
  
  
root@ibof-target:IBOF_HOME/lib/spdk-20.10/scripts# ./rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem1 -t tcp -a 10.100.11.20 -s 1158
```

In the above example, the NVM subsystem called "nqn.2019-04.pos:subsystem1" has been configured to listen on (10.100.11.20, 1158) and use TCP transport. If you miss this step, POS wouldn't be able to mount POS volumes even though it could create new ones. 
At this point, you should be able to retrieve the configured NVM subsystem like in the following:

Retrieve NVM subsystem information
```bash
root@ibof-target:IBOF_HOME/lib/spdk-20.10/scripts# ./rpc.py nvmf_get_subsystems
[
  {
    "nqn": "nqn.2014-08.org.nvmexpress.discovery",
    "subtype": "Discovery",
    "listen_addresses": [],
    "allow_any_host": true,
    "hosts": []
  },
  {
    "nqn": "nqn.2019-04.pos:subsystem1",
    "subtype": "NVMe",
    "listen_addresses": [
      {
        "transport": "TCP",
        "trtype": "TCP",
        "adrfam": "IPv4",
        "traddr": "10.100.11.20",
        "trsvcid": "1158"
      }
    ],
    "allow_any_host": true,
    "hosts": [],
    "serial_number": "POS00000000000001",
    "model_number": "POS_VOLUME_EXTENSION",
    "max_namespaces": 256,
    "namespaces": []
  }
]
```

### Step 8. Create POS Volume

This step is to create a logical entry point of the target side IO which will be shown as a namespace in an NVM subsystem. POS volume is wrapped as a "bdev" and can be attached to a particular NVM subsystem. "bdev" is a block device abstraction offered by SPDK library. 

Create a volume
```bash
ibof@ibof-target:IBOF_HOME/bin$ ./cli volume create --name vol1 --size 1073741824 --maxiops 0 --maxbw 0 --array POSArray
  
  
Request to Poseidon OS
    xrId        :  05fcfcc1-2f8e-11eb-9cac-005056adb61a
    command     :  CREATEVOLUME
    Param       :
{
    "name": "vol1",
    "array": "POSArray",
    "size": 1073741824
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
ibof@ibof-target:IBOF_HOME/bin$ ./cli volume list --array POSArray
  
  
Request to Poseidon OS
    xrId        :  3e3ea57c-2f8e-11eb-a2ad-005056adb61a
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
            "remain": "1.073741824GB (1073741824B)",
            "status": "Unmounted",
            "total": "1.073741824GB (1073741824B)"
        }
    ]
}
```
Please note that the initial status of POS volume is Unmounted. 

### Step 9. Mount POS Volume

This is to make a particular POS volume ready to perform IO. After this step, POS volume is attached as bdev to an NVM subsystem and seen as an NVM namespace. 


```bash
ibof@ibof-target:IBOF_HOME/bin$ ./cli volume mount --name vol1 --array POSArray
  
Request to Poseidon OS
    xrId        :  b4224be2-2f97-11eb-bd11-005056adb61a
    command     :  MOUNTVOLUME
    Param       :
{
    "name": "vol1",
    "array": "POSArray"
}
 
ibof@ibof-target:IBOF_HOME/bin$ ./cli volume list --array POSArray
  
  
Request to Poseidon OS
    xrId        :  ea8e0b0c-2f97-11eb-9d8a-005056adb61a
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
root@ibof-target:IBOF_HOME/lib/spdk-20.10/scripts# ./rpc.py nvmf_get_subsystems
[
  {
    "nqn": "nqn.2014-08.org.nvmexpress.discovery",
    "subtype": "Discovery",
    "listen_addresses": [],
    "allow_any_host": true,
    "hosts": []
  },
  {
    "nqn": "nqn.2019-04.pos:subsystem1",
    "subtype": "NVMe",
    "listen_addresses": [
      {
        "transport": "TCP",
        "trtype": "TCP",
        "adrfam": "IPv4",
        "traddr": "10.100.11.20",
        "trsvcid": "1158"
      }
    ],
    "allow_any_host": true,
    "hosts": [],
    "serial_number": "POS00000000000001",
    "model_number": "POS_VOLUME_EXTENSION",
    "max_namespaces": 256,
    "namespaces": [
      {
        "nsid": 1,
        "bdev_name": "bdev0",
        "name": "bdev0",
        "uuid": "06d53d40-33b7-4d05-be20-58220496a43d"
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
ibof@ibof-target:IBOF_HOME/bin$ ./cli volume unmount --name vol1 --array POSArray
  
  
Request to Poseidon OS
    xrId        :  319c6a9f-2fb4-11eb-a9bd-005056adb61a
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
  
  
ibof@ibof-target:IBOF_HOME/bin$ ./cli volume list --array POSArray
  
  
Request to Poseidon OS
    xrId        :  352d6cce-2fb4-11eb-ab45-005056adb61a
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
            "remain": "1.073741824GB (1073741824B)",
            "status": "Unmounted",
            "total": "1.073741824GB (1073741824B)"
        }
    ]
}
```
```bash
Please note that the status of the POS volume has changed from "Mounted" to "Unmounted".
```

### Step 11. Delete POS Volume

```bash
ibof@ibof-target:IBOF_HOME/bin$ ./cli volume delete --name vol1 --array POSArray
  
  
Request to Poseidon OS
    xrId        :  95e706cc-2fb4-11eb-b82f-005056adb61a
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
  
  
ibof@ibof-target:IBOF_HOME/bin$ ./cli volume list --array POSArray
  
  
Request to Poseidon OS
    xrId        :  97b9746e-2fb4-11eb-b2f1-005056adb61a
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
ibof@ibof-target:IBOF_HOME/bin$ ./cli array unmount --name POSArray
  
  
Request to Poseidon OS
    xrId        :  d32e2fbd-2fb4-11eb-91a3-005056adb61a
    command     :  UNMOUNTARRAY
  
  
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
  
  
ibof@ibof-target:IBOF_HOME/bin$ ./cli array info --name POSArray
  
  
Request to Poseidon OS
    xrId        :  3f30be48-2f8c-11eb-9afa-005056adb61a
    command     :  ARRAYINFO
  
  
Response from Poseidon OS
    Code         :  0
    Level        :  INFO
    Description  :  Success
    Problem      :
    Solution     :
    Data         :
 {
    "name":"POSArray", 
    "state":"OFFLINE",
    "situation":"DEFAULT",
    "rebuilding_progress":0,
    "capacity":0,
    "used":0,
    "devicelist":[
        {
            "type":"BUFFER",
            "name":"uram0"
        },
        {
            "type":"DATA",
            "name":"unvme-ns-0"
        },
        {
            "type":"DATA",
            "name":"unvme-ns-1"
        },
        {
            "type":"DATA",
            "name":"unvme-ns-2"
        },
        {
            "type":"SPARE",
            "name":"unvme-ns-3"
        }
    ]
}

```

### Step 13. Delete POS Array

```bash
ibof@ibof-target:IBOF_HOME/bin$ ./cli array delete --name POSArray
 
 
Request to Poseidon OS
    xrId        :  39434590-32d4-11eb-ae61-005056adb61a
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
 
 
ibof@ibof-target:IBOF_HOME/bin$ ./cli array info --name POSArray
 
 
Request to Poseidon OS
    xrId        :  3cb6ebf0-32d4-11eb-bba1-005056adb61a
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
ibof@ibof-target:IBOF_HOME/bin$ ./cli system exit
 
 
Request to Poseidon OS
    xrId        :  81f12c3c-32d4-11eb-8a99-005056adb61a
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
ibof@ibof-target:IBOF_HOME/bin$ ps -ef | grep poseidonos | grep -v grep
```



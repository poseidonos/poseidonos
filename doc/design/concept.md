# POS concepts
This section will walk you through the basic concepts and terms to understand Poseidon OS (POS) with a few illustrative examples of the network architecture and software stack.

## What is Poseidon OS (POS)?
Poseidon OS (POS) is a light-weight storage OS that offers the best performance and valuable features over storage network. POS exploits the benefit of NVMe SSDs by optimizing storage stack and leveraging the state-of-the-art high speed interface. Also, it implements software-defined storage capabilities to meet the various needs of storage applications in a data center. With POS, industries can easily build Composable Disaggregated Infrastructure in their data centers. The latest version of POS is 0.9.2 and released at https://github.com/poseidonos/poseidonos.

The key characteristics of POS are as follows: 

**Highly optimized for low-latency & high-throughput NVMe devices**

The performance characteristics of an NVMe device differ a lot from those of other types of devices (e.g., SATA HDD/SSD, ...) and break assumptions in a traditional storage stack. For example, the traditional I/O path based on interrupt and soft IRQ could suffer from the context switch overhead, which used to work efficiently for disk-based storage system. To fully exploit the benefit of using fast storage devices, POS implements highly optimized I/O path with minimum software overhead and maximum parallelism. 

**Supporting NVMe-over-Fabrics interface over RDMA and TCP**

POS is capable of exposing block devices over network. This helps cloud vendors to separate their applications from the physical topology of storage devices. By avoiding tight coupling between compute and storage resources, the vendors should be able to deploy and balance their application workloads more flexibly. 

**Running as user-level application (vs. kernel-level)**

One of the key design decisions is to run POS as user-level application. The rationale behind it is to fully control the I/O path and avoid wasting CPU cycles required by general-purpose OS activities. The following figure illustrates what types of software overheads exist in Kernel Device Driver (KDD) and how they could be eliminated in User Device Driver (UDD).

**Virtualizing storage resources**

POS offers virtualized block devices that provide valuable features as well as block read/write functionality: capacity elasticity, performance throttling, data protection (RAID), and more. Customers could benefit from the features for free transparently. The following figure illustrates how POS (represented as "Reference Software") plays a role in storage virtualization. POS sits between initiator(s) and NVMe device(s) and remaps, duplicates, and/or buffers I/O requests to implement the features.

## NVMe-over-Fabrics (NVMe-oF) Interface
### Why New Interface?
NVMe-oF has emerged to break through the scaling limitations of PCIe-attached NVMe. NVMe-oF allows storage applications to talk to remote NVMe devices over network and consequently unblocks those applications from the physical limitation on the number of PCIe lanes a single host can have. NVMe-oF performs encapsulation/decapsulation at each layer of the software stack to maintain end-to-end NVMe semantics across a range of topology. The following illustrates how an NVMe command travels from client (hosts) to server SSDs through NVMe-oF protocol. 

### Network Transport (TCP vs. RDMA)
The network fabric used for NVMe-oF can be several types (i.e., Fibre Channel, RDMA, and TCP) according to the current NVMe-oF 1.1 Specification.  POS currently supports TCP and RDMA (RoCE v2). We should be aware of pros and cons of each transport type, so that we could make the best decision under various constraints. 

**TCP transport** relies on commodity network hardware including switches, Ethernet NICs, and cables. Datacenter can utilize their existing TCP network with NVMe/TCP. Operating TCP network infrastructure is relatively cheaper and also has wider community support. However, we can't avoid going through kernel stack that uses TCP sockets and handling network interrupts, which could affect the latency of an I/O request, significantly. TCP transport also generally requires more CPU resources than RDMA does.

**RDMA transport** offloads the entire network processing stack to its custom hardware and saves host CPU cycles to process packets. It can also achieve extremely low latency by bypassing the kernel stack, resulting in better performance. On the other hand, the cost of RDMA-specific switches, NICs, and cables is known to be more expensive than TCP-based commodity hardwares because RDMA relies on a lossless network. 

The following figure illustrates the network I/O path of TCP and RDMA. The major difference is whether Kernel Space is involved in I/O or not. As described before, a TCP packet goes through Kernel Space with multiple context switches, while RDMA packet bypasses the intermediate layers and directly gets to the User Space.

### POS as an NVMe-oF Target

PSD (PoSeiDon) server is a (Samsung & Inspur)-engineered storage server that runs POS. POS enables initiator to access PSD server (specifically, NVM subsystem) as if it is a single or multiple local block device(s), while in reality the device is remotely mapped to multiple physical NVMe SSDs on PSD server. POS internally manages a pool of NVMe SSDs called POS array to construct volume(s) and expose the devices as a namespace to the initiator side in accordance with NVMe-oF specification. 

The following network diagram shows how PSD server can be networked to support multiple initiators and NVM subsystems. PSD server instantiates two NVM subsystems, where the first one (called "0") is assigned with a single controller (called "A") and the second one (called "1") has two controllers (called "A" and "B"). As in the example, an NVMe-oF block device on the Initiator side is always mapped to a single POS volume on the target side, while a single POS volume can be connected by multiple controllers and initiators. The types of available network transport currently are TCP and RDMA (RoCE v2). In addition to the user I/O path, there is a separate network path that allows a system administrator to perform storage provisioning, monitoring, troubleshooting, testing, and etc. This management network is only accessible locally through Unix Domain Socket.

## Device and Array
POS internally manages various types of storage resources to support virtualization. Device and Array are essential building blocks to implement the feature. In this section, we will explore how they are abstracted out to support various storage use cases and what kinds of administrative knowledge is there to understand the internals of the POS system. 

### What Is POS Array?
POS array is a user-defined collection of physical storage devices and a basic unit to which storage administrator could issue commands such as status-check, create-volume, delete-volume, mount, or unmount. The same storage device cannot be owned by two different arrays. In this sense, you could think of POS array as a way to group a set of physical storage resources exclusively. 

POS array can contain 1 or more POS volumes, each of which can be exposed to initiator(s) as a block device. POS array enforces the same data protection policy across its volumes. 

In the following figure, POS manages two POS arrays, "A" and "B", each exposing two POS volumes. Initiator gets connected to all those 4 POS volumes over NVMe-oF, converts block requests from applications into NVMe-oF packets, and sends/receives them to/from POS volumes "A", "B", "C", and "D". POS is responsible for translating the destination address of initiator's request into a list of (SSD id, Logical Block Addresses).

### Creating POS Array
Storage administrator may want to manage multiple POS arrays to meet various Service-Level Agreement (SLA) requirements.  

- POS can have up to 8 arrays in total.
- Applications having strict performance SLA should run on a dedicated, isolated set of SSD devices, because they should not be interfered by the other applications.
- Certain application has strict durability SLA and needs to have higher redundancy than RAID 5. 
- The failure domain of one application needs to be isolated from that of other application. Even if certain POS array gets data loss due to bugs, multiple SSD failures, corruptions, and etc, the issue shouldn't propagate to other POS arrays. 
- The lifespans of SSDs should be spread out sufficiently such that we shouldn't have to replace all of them at the same time. Having separate arrays could isolate I/O workloads and effectively spread the lifespans compared to the case that a single array serves all workloads in a mixed fashion. 


To create a new array, the following information needs to be provided for POS:

- Array name: POS needs to be able to uniquely identify the target POS array. 
- Buffer device: POS accumulates writes in a sequential fashion in a given "buffer" device (normally, NVRAM) to improve write performance and latency.
- Data devices: POS stores both user data and metadata in "data" devices. POS internally maintains block mapping to make the best use of SSDs, e.g., by striping across data devices and chunking within a data device.
- Spare device(s): When POS detects a faulty device, it rebuilds missing data onto a spare device by calculating RAID parity. Spare devices can be dynamically attached/detached even when POS array is mounted.
- RAID type: POS protects user data with RAID. Currently only RAID5 is supported.


POS validates if user-supplied inputs are well-formed and complies with system constraints:

- The length of an array name can be up to 63 characters. The possible character is [a-zA-Z0-9_-].
- POS array must have exactly one buffer device by design.
- The minimum number of data devices is 3. This is required by RAID 5.
- The name of a buffer/data/spare must be picked up from SPDK runtime. Getting started describes what those names look like and how to retrieve them by using POS CLI. 
- The capacity of a buffer device must be equal to or larger than "(128 MB * # of data devices) + 512 MB". 
- The maximum number of data and spare devices in total is 32. 
- The capacity of a single NVMe SSD must be between 20 GB and 32 TB.

### Partition
POS introduces a logical storage resource called POS partition that internally provides multi-devices abstraction for other components. Every "data" device within an POS array is divided into three areas for different purposes. The collection of the same area from all "data" devices is defined as POS partition. Hence, POS array has three partitions, each of which spanning across all data devices. POS partition enables other software components to access multiple data devices in parallel without detailed knowledge about their physical layout. 

The types of POS partition are as follows:

- Master Boot Record (MBR) partition stores array configuration. If POS or underlying host gets restarted for any reason, POS would retrieve the previous array information from MBR partition and reconstruct its internal data structures in memory.
- Meta-data partition manages logical-to-physical mappings and write operations in the partition to support virtualization and consistency. 
- User-data partition stores actual user content transferred from initiator(s).


The following figure illustrates the layout of the partitions within POS array.

The layout of three areas within a data device looks exactly the same. We name them as MBR, metadata, and userdata "partition area" respectively. Here is how POS calculates the size of each partition area and the capacity of POS array:

- MBR partition area = 256 KB
- Metadata partition area = 2% * the raw size of a data device.
- Userdata partition area = the raw size of a data device - MBR partition area - metadata partition area
- Userdata partition area reserves 10% of its size for Over Provisioning area. 
- Effective userdata partition area = userdata partition area * 0.9
- The capacity of POS array = effective userdata partition area * (# of data devices - # of parity devices)

### RAID
POS implements data redundancy at POS partition level. Parity device in POS is defined as a device that contains device recovery information. 

- Metadata partition is protected by RAID 1. The metadata partition area is divided into multiple chunks, each of which being fully mirrored. Hence, the number of parity devices equals to 50% of the total number of data devices in POS array.
- Userdata partition is protected by RAID 5. The userdata partition area is divided into multiple chunks, each of which containing either data or parity bits. POS stores parity chunk in a round-robin fashion, so keeps one parity device effectively.

When POS detects a data device failure, it automatically enters "degraded" mode, which is a fallback mode that continues general array operations but with potential performance degradation caused by 1) decreased read parallelism and 2) resource contention due to RAID rebuild operation. 

POS conditionally initiates data construction procedure, called as "RAID rebuild", to replace a failed device with a new one. In degraded mode, POS checks periodically whether there is an available spare device and proceeds to rebuild if there is any. Otherwise, POS array will remain in degraded mode until a spare device is newly added. The RAID rebuild doesn't block user I/Os from being processed, although it may impact on the performance of user I/Os. Please note that RAID rebuild generates intense internal I/Os to copy blocks between spare and data devices. POS offers a CLI command called "perf_impact" for a storage administrator to be able to adjust the level of the intensity and find a balance between fast recovery and user's perf SLA. 

### State Transition Diagram
Upon internal/external events, POS may switch from one state to another and adjust its behavior. For example, if POS is in PAUSE state to recover from journals, it will block any user I/Os during the period. In this section, we will explain the states that POS could enter and what their implications are. 

#### Types of Array State
Array is always in one of POS states in the following table.

|State|Description|
|---|---|
|OFFLINE|POS array has been created or loaded, but not mounted yet. |
|STOP|POS array is running into an unusual situation that it cannot be corrected by itself. POS array becomes unavailable to avoid further data loss/corruption. For example, if the number of failed devices exceeds the level of fault tolerance (e.g., 2 failures in RAID 5-protected POS partition), a lost device cannot be restored.|
|NORMAL|POS array is operating normally and able to serve user I/Os without performance impact.|
|BUSY|POS array is handling both internal I/Os and user I/Os. The user I/O performance could be potentially degraded due to resource contention.|
|PAUSE|POS array is blocking user I/Os to change online - offline state.|

#### Types of Situation
Situation indicates the detailed explanation of POS array state. Situation is always mapped to a single state, while State can be mapped to multiple Situations. 

|Situation|Description|State|
|---|---|---|
|DEFAULT|Initial state of POS array|OFFLINE|
|NORMAL|POS array is operating normally and able to serve user I/Os without performance degradation or blockers|NORMAL|
|TRY_MOUNT|POS array is being mounted. It is retrieving configuration information and checking integrity|PAUSE|
|DEGRADED|One storage device has failed, resulting in RAID rebuild|BUSY|
|TRY_UNMOUNT|POS array is performing "unmount" by blocking both user and internal I/Os|PAUSE|
|JOURNAL_RECOVERY|During "mount" operation, POS array has detected unapplied journal logs from meta-data partition and started recovery operation|PAUSE|
|REBUILDING|RAID rebuild operation is in progress|BUSY|
|FAULT|POS array is not able to proceed with RAID rebuild because the number of failed devices has exceeded the level the array can tolerate|STOP|

#### State Transition Diagram
The following diagram illustrates what conditions trigger a state transition of POS array.

## Volumes
### What Is POS Volume?
POS volume is storage resource visible as a block device to a client host. Internally, POS volume is an NVM namespace and attached to one of the NVM subsystems on a target. When POS volume is mounted, the corresponding NVM namespace allows incoming connection and is accessible to initiator(s). When the connection is established, NVM subsystem creates a new controller, from which point the initiator can send/receive block requests over the connection. This process is illustrated in the following figure.

Here are a few constraints on the relationship among the components:

- POS volume cannot be shared by multiple NVM namespaces. 
- NVM controller is associated with one host at a time.

The following figure elaborates more on how block devices on an initiator are mapped to NVM namespaces on a target. The target is exposing two NVM namespaces from NVM subsystem 0 and another two from NVM subsystem 1. Each NVM namespace is called as POS volume and abstracted as a block device on the initiator. The naming convention of a standard NVMe-oF block device is, /dev/nvme{subsystem number}n{namespace number}. For example, /dev/nvme1n1 will be mapped to (NVM subsystem 1, NVM namespace 1) on a target. 

```bash
Node             SN                   Model                                    Namespace Usage                      Format           FW Rev
---------------- -------------------- ---------------------------------------- --------- -------------------------- ---------------- --------
/dev/nvme0n1     IBOF00000000000001   IBOF_VOLUME_EXTENTION                    1           2.15  GB /   2.15  GB    512   B +  0 B   19.10
/dev/nvme0n2     IBOF00000000000001   IBOF_VOLUME_EXTENTION                    2           2.15  GB /   2.15  GB    512   B +  0 B   19.10
/dev/nvme1n1     IBOF00000000000002   IBOF_VOLUME_EXTENTION                    1           2.15  GB /   2.15  GB    512   B +  0 B   19.10
/dev/nvme1n2     IBOF00000000000002   IBOF_VOLUME_EXTENTION                    2           2.15  GB /   2.15  GB    512   B +  0 B   19.10
```

### Constraints on POS Volume Creation
POS validates user-supplied inputs when creating POS volume. The properties of POS volume must satisfy the following constraints:

- The size must be multiples of 1 MB.
- The minimum size is 1 MB. 
- The maximum size can be as large as the free space of POS array. 
- The size can be specified in units of Byte, Mega Byte, Giga Byte.
- The length of a volume name must be between 2 and 255 (inclusive). 
- Any whitespace(s) in the front or the end of a volume name is trimmed. 
- Only the following character set is allowed for a volume name: [a-zA-Z0-9_-]
- The volume name must be unique within POS array.
- The maximum number of POS volumes is limited to 256 for a single POS array.
- POS array must be in "NORMAL" (i.e., successfully mounted) or "BUSY" state to be able to create new POS volume.

### Constraints on Throttling POS Volume Performance
Storage administrator can choose to set performance limit on per-volume basis. The maximum bandwidth (BW) or IOPS value can be provided during volume creation or configured dynamically afterwards. This configuration affects both read/write performance of a volume. 

POS validates the configuration and uses it only when they are within an expected range. Otherwise, POS may cut it down or pick up a smaller one. Here are a few contracts and constraints: 

- If the sum of (BW, IOPS) pairs from all POS volumes exceeds the capacity of POS performance, POS chooses a smaller value than what is provided by a user.
- If both IOPS and BW are set, POS chooses whichever is satisfied first for throttling. 
- BW must be between 10 ~ 17592186044415(UINT64_MAX / 1024 / 1024) and in the unit of MiB.
- If BW is null (not given) or 0, POS will not impose any BW limit on a volume.
- IOPS must be between 10 ~ 18446744073709551(UINT64_MAX / 1000) and in the unit of KIOPS.
- If IOPS is null (not given) or 0, POS will not impose any IOPS limit on a volume.
- POS array must be in "NORMAL" (i.e., successfully mounted) or "BUSY" state to be able to configure a volume with (BW, IOPS).

### Mounting POS Volume
"Mount" operation establishes NVMe-oF connection between an initiator and a target. If storage administrator does not specify an NVM subsystem to attach POS volume to, POS chooses the next available NVM subsystem in a round-robin fashion. 

### Unmounting Volume
"Unmount" operation detaches POS volume from its corresponding NVM subsystem, stopping I/O requests between an initiator and a target.

## Data Persistence and Consistency
### Overview
In this section, we describe how POS persists user data and guarantees metadata consistency to deal with crash failures. We also discuss the limitation of POS at the time of writing. This will help to set the right expectation about the level of persistence in the current software release and hardware configuration. POS adopts software-only approach for persistence and consistency, making it highly portable and flexible across different types of hardware. 

### Data Persistence and Consistency Issues
When user data arrives at POS, it is first accumulated in internal storage resource called "write buffer" and then later flushed into NVMe SSDs; this is one of the key I/O path optimizations. The buffer is located within a buffer device of POS array, hence inheriting the same persistence of the buffer device. POS sends back write acknowledgement to an initiator right after the buffering, without waiting for the result of the flushing. This technique effectively reduces write latency.

Sometime, however, it is possible that POS crashes due to power failure before flushing the user data into NVMe SSDs, although POS sent back the write acknowledgement to the initiator. To deal with this situation, POS should be able to handle the following issues:

- Persistence issue: If a buffer device is allocated from volatile media (e.g., DRAM) for any reason, the write buffer would be lost after a server reboot. In this case, an initiator would think its data should be persistent on POS since it got write ack already, but in fact has lost its data. 
- Consistency issue: POS may have been in the middle of updating its internal metadata, resulting in partial updates. After a server reboot, POS might see inconsistent image of metadata such as dangling pointers or orphans. 

POS offers a few workarounds to deal with such expected issues.

### How To Keep Data Persistent and Metadata Consistent
#### Issue NVM Flush Command
NVMe specification explains what to expect from NVM flush command: "The Flush command shall commit data and metadata associated with the specified namespace(s) to nonvolatile media". Provided that POS volume is implemented as an NVM namespace, it is supposed to copy all dirty blocks to NVMe SSDs synchronously upon receiving the command. The feature was enabled/disabled through build option in earlier version, but it has been configurable through config file change (at /etc/pos/pos.conf) since 0.9.2. It is not enabled by default yet. This does not solve the sudden crash problem perfectly, but can help to reduce the data loss window if it is run on a reasonably-short interval. 

#### Unmount POS Array Gracefully
We offer the best practice to shut down POS array, which is called "graceful unmount". The word "graceful" means that user application prepares for a stop and should not observe any errors during the procedure. The following steps guarantee that all in-transit writes are successfully done to NVMe SSDs consistently:

1. Stop receiving user I/Os from the initiator by stopping user application and unmounting the file system
2. Unmount POS volume from the target
3. Unmount POS array from the target
4. Stop POS at the target

This still does not solve the sudden crash problem, but can help with shutting down POS reliably for any maintenance job, e.g., OS patch, library upgrade, POS configuration change, and etc. 

#### Enable Journal
With enabling journal and configuring non-volatile write buffer, both persistence and consistency can be achieved transparently without any explicit user commands such as NVM flush and graceful exit. All write-ack'd data in the buffer would be eventually flushed to NVMe SSDs even in case of sudden crashes. Also, all metadata updates are recorded and replayed to implement all-or-nothing semantics, achieving the consistency. This feature is experimental at the time of writing and will be included in a later release. 

```
With volatile write buffer, it is required to issue NVM flush command explicitly or exit Poseidon OS gracefully to store user data and meta data safely.
```
```
With non-volatile write buffer, user data consistency can be guaranteed by enabling journal.
```

### Case Study for Write Buffer Configuration
#### Case 1: Using ramdisk for write buffer
Storage administrator may want to allocate write buffer from ramdisk, not caring much about the persistence of user data. For example, if the user data is derived from other data sources (e.g., cached view, external sorting, table merge space, AI learning dataset, ...) and can be rebuilt in the worst case, the admin may favor the DRAM's performance over other benefits. In this configuration, both user data and journal logs in write buffer are not guaranteed to be safely stored in NVMe SSDs upon failures. 

With journal feature disabled, it is desirable to issue NVM flush command on a periodic basis and/or perform graceful exit to ensure persistence and consistency.

With journal feature enabled, the metadata consistency is guaranteed but the data can be lost on power or kernel failure. User data can be recovered from POS's internal failures as long as the write buffer is restored before POS starts up. 

#### Case 2: Using NVRAM for write buffer
Storage administrator may want to store system-of-records (SOR) in POS and favor persistence the most. Then, he/she could allocate write buffer from NVRAM so that all write ack'd data could tolerate sudden crashes. 

With journal feature disabled, the only way to guarantee metadata consistency is for a user to issue NVM flush command periodically and perform graceful exit. Given that sudden crash could occur at any time point, this configuration is still vulnerable to power failures; POS's internal metadata has a small chance of being inconsistent. 

With journal feature enabled, the metadata consistency is guaranteed as well as the data persistence. 

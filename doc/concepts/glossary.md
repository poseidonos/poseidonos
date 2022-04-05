| **No** | **Terminology**| **Description** |
|:------:|:-------:|:-------:|
|	1	|	Poseidon Server	|	An NVMe-oF reference hardware server made by Samsung and Inspur.	|
|	2	|	Poseidon OS	|	A storage OS to manage a Poseidon server and provide storage service.	|
|	3	|	IBoF	|	An internal code name for the Poseidon project, which stands for intelligent bunch of flash.	|
|	4	|	Array	|	An abstract unit of storage pool, which is a set of data device, spare device, and write-buffer. A volume can be created from an array. It provides RAID mechanism for data protection. It can be seen as a subsystem in terms of NVMe-oF.	|
|	5	|	Block	|	The basic unit of I/O in PoseidonOS. Its size is typically 4KB.	|
|	6	|	Chunk	|	Set of blocks. A chunk typically includes 64 blocks and its size is the same as that of an SSD device.	|
|	7	|	Stripe	|	A set of chunks. For example, when an array consists of 16 data devices (chunks), a strip has a size of 16 chunks.	|
|	8	|	Segment	|	A set of stripes. Allocator and GC use this unit of user I/O. A segment typically has 1024 stripes. Size of a segment is aligned to that of erase block of the SSD devices.	|
|	9	|	Partition	|	A logical region of an array that is partitioned for purposes. Write buffer device has journal partitions and write buffer partitions, and data device has an MBR partition, a journal partition, a meta-data partition, and user-data partitions.	|
|	10	|	Initiator	|	An endpoint that initializes and sends I/O requests.	|
|	11	|	Target	|	An endpoint that waits for the commands from the initiator and provides data I/O service.	|
|	12	|	Backend I/O	|	A data I/O operations from write buffer to SSD stripes.	|
|	13	|	Frontend I/O	|	User I/O operation to be written in Write buffer.	|
|	14	|	Poseidon-GUI	|	GUI-based dashboard that receives and processes user requests.	|
|	15	|	Poseidon-CLI	|	A text-based user interface that receives and processes a user's request.	|
|	16	|	Checkpoint (CP)	|	A fault-tolerance technique that includes storing a state of a program at a specific time and restarting the program from the state. The states in PoseidonOS are dirty data and meta data, and they are flushed at some time.	|
|	17	|	Metadata	|	Data that provides the information about a data.	|
|	18	|	Device Mornitoring Daemon	|	A daemon that periodically monitors devices in the Poseidon system to notify PoseidonOS when a device is attached or detached. A device is an abstraction of a physical storage that can store data. Devices are data device, spare device, and URAM.	|
|	19	|	Garbage Collectior (GC)	|	The operation of moving valid blocks to allocate free segments.	|
|	20	|	Journal	|	A circular log data structure that records the changes in the file system. When the file system causes crashes and errors, data can be recovered from the journal data.	|
|	21	|	Logical Block Address	|	An address for a block in storage systems.	|
|	22	|	In-place update	|	An overwrite scheme that update the data for the given address.	|
|	23	|	Log-structured writing	|	Append only writing considering characteristics of SSD excellent for continuous writing.	|
|	24	|	Management Stack	|	Software stack that provides an interface to manage the Poseidon system to users or 3rd party services. It provides Poseidon-GUI and RESTful API.	|
|	25	|	RAID10	|	RAID10 is a Redundant Array of Independent Disk (RAID) configuration that combines disk mirroring for physical data copying and disk striping. In POS, Metadata partition is protected by RAID 10. The metadata partition area is divided into multiple chunks, each of which being fully mirrored. Hence, the number of parity devices equals to 50% of the total number of data devices in POS array.	|
|	26	|	RAID5	|	RAID 5 is a redundant array of independent disks configuration that uses disk striping with rotated parity disk. In POS, user data partition is protected by RAID 5 as the default. The user data partition area is divided into multiple chunks, each of which containing either data or parity bits. POS stores parity chunk in a round-robin fashion, so keeps one parity device effectively. 	|
|	27	|	Spare Device	|	A device to replace a data device when the data device is failed. An array consists of data and array devices using a RAID scheme.	|
|	28	|	Subsystem	|	Component that processes NVMe commands. An NVMe subsystem includes one or more NVMe controller, one or more port, and one or more non-volatile memory.	|
|	29	|	Transport	|	A protocol layer that provides a reliable transfers of data, command, and response between source and destination.	|
|	30	|	Ubio	|	Ubio is an abstraction of an I/O request used throughout Frontend and Backend I/O path. Any Module needs I/O service can make Ubios and pass them to a particular UBlockDevice (As-Is) through a proper IOWorker.	|
|	31	|	Volume	|	Logical storage exposed to users as a block device.	|
|	32	|	Write Buffer	|	A cache that stores data before actually writing it to the data devices. It is typically created in DRAM or NVRAM.	|

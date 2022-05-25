# Data Persistence and Consistency
## Overview
In this section, we describe how POS persists user data and guarantees metadata consistency to deal with crash failures. We also discuss the limitation of POS at the time of writing. This will help to set the right expectation about the level of persistence in the current software release and hardware configuration. POS adopts software-only approach for persistence and consistency, making it highly portable and flexible across different types of hardware. 

## Data Persistence and Consistency Issues
When user data arrives at POS, it is first accumulated in internal storage resource called "write buffer" and then later flushed into NVMe SSDs; this is one of the key I/O path optimizations. The buffer is located within a buffer device of POS array, hence inheriting the same persistence of the buffer device. POS sends back write acknowledgement to an initiator right after the buffering, without waiting for the result of the flushing. This technique effectively reduces write latency.

Sometime, however, it is possible that POS crashes due to power failure before flushing the user data into NVMe SSDs, although POS sent back the write acknowledgement to the initiator. To deal with this situation, POS should be able to handle the following issues:

- Persistence issue: If a buffer device is allocated from volatile media (e.g., DRAM) for any reason, the write buffer would be lost after a server reboot. In this case, an initiator would think its data should be persistent on POS since it got write ack already, but in fact has lost its data. 
- Consistency issue: POS may have been in the middle of updating its internal metadata, resulting in partial updates. After a server reboot, POS might see inconsistent image of metadata such as dangling pointers or orphans. 

POS offers a few workarounds to deal with such expected issues.

## How To Keep Data Persistent and Metadata Consistent
### Issue NVM Flush Command
NVMe specification explains what to expect from NVM flush command: "The Flush command shall commit data and metadata associated with the specified namespace(s) to nonvolatile media". Provided that POS volume is implemented as an NVM namespace, it is supposed to copy all dirty blocks to NVMe SSDs synchronously upon receiving the command. The feature was enabled/disabled through build option in earlier version, but it has been configurable through config file change (at /etc/pos/pos.conf) since 0.9.2. It is not enabled by default yet. This does not solve the sudden crash problem perfectly, but can help to reduce the data loss window if it is run on a reasonably-short interval. 

### Unmount POS Array Gracefully
We offer the best practice to shut down POS array, which is called "graceful unmount". The word "graceful" means that user application prepares for a stop and should not observe any errors during the procedure. The following steps guarantee that all in-transit writes are successfully done to NVMe SSDs consistently:

1. Stop receiving user I/Os from the initiator by stopping user application and unmounting the file system
2. Unmount POS volume from the target
3. Unmount POS array from the target
4. Stop POS at the target

This still does not solve the sudden crash problem, but can help with shutting down POS reliably for any maintenance job, e.g., OS patch, library upgrade, POS configuration change, and etc. 

### Enable Journal
With enabling journal and configuring non-volatile write buffer, both persistence and consistency can be achieved transparently without any explicit user commands such as NVM flush and graceful exit. All write-ack'd data in the buffer would be eventually flushed to NVMe SSDs even in case of sudden crashes. Also, all metadata updates are recorded and replayed to implement all-or-nothing semantics, achieving the consistency. This feature is experimental at the time of writing and will be included in a later release. 

```
With volatile write buffer, it is required to issue NVM flush command explicitly or exit Poseidon OS gracefully to store user data and meta data safely.
```
```
With non-volatile write buffer, user data consistency can be guaranteed by enabling journal.
```

## Case Study for Write Buffer Configuration
### Case 1: Using ramdisk for write buffer
Storage administrator may want to allocate write buffer from ramdisk, not caring much about the persistence of user data. For example, if the user data is derived from other data sources (e.g., cached view, external sorting, table merge space, AI learning dataset, ...) and can be rebuilt in the worst case, the admin may favor the DRAM's performance over other benefits. In this configuration, both user data and journal logs in write buffer are not guaranteed to be safely stored in NVMe SSDs upon failures. 

With journal feature disabled, it is desirable to issue NVM flush command on a periodic basis and/or perform graceful exit to ensure persistence and consistency.

With journal feature enabled, the metadata consistency is guaranteed but the data can be lost on power or kernel failure. User data can be recovered from POS's internal failures as long as the write buffer is restored before POS starts up. 

### Case 2: Using NVRAM for write buffer
Storage administrator may want to store system-of-records (SOR) in POS and favor persistence the most. Then, he/she could allocate write buffer from NVRAM so that all write ack'd data could tolerate sudden crashes. 

With journal feature disabled, the only way to guarantee metadata consistency is for a user to issue NVM flush command periodically and perform graceful exit. Given that sudden crash could occur at any time point, this configuration is still vulnerable to power failures; POS's internal metadata has a small chance of being inconsistent. 

With journal feature enabled, the metadata consistency is guaranteed as well as the data persistence. 

- [- _**VSAMapFlushedDirtyPageCount**_](#--vsamapflusheddirtypagecount)
- [**MetaFs**](#metafs)
  - [_**NormalShutdown**_](#normalshutdown)
  - [_**PendingMioCount**_](#pendingmiocount)
  - [_**PendingMpioCount**_](#pendingmpiocount)
- [**Volume**](#volume)
  - [_**read_iops**_](#read_iops)
  - [_**read_rate_bytes_per_second**_](#read_rate_bytes_per_second)
  - [_**read_latency_mean_ns**_](#read_latency_mean_ns)
  - [_**read_latency_max_ns**_](#read_latency_max_ns)
  - [_**write_iops**_](#write_iops)
  - [_**write_rate_bytes_per_second**_](#write_rate_bytes_per_second)
  - [_**write_latency_mean_ns**_](#write_latency_mean_ns)
  - [_**write_latency_max_ns**_](#write_latency_max_ns)
- [**Array**](#array)
  - [_**ArrayStatus**_](#arraystatus)
  - [_**RebuildCount**_](#rebuildcount)
  - [_**GarbageCollectorStatus**_](#garbagecollectorstatus)
- [**Meta**](#meta)
  - [**Allocator**](#Allocator)
    - [_**FreeSegmentCount**_](#freesegcnt)
    - [_**ContextManagerPendingIoCount**_](#ctxmngpendingiocnt)
    - [_**GCVictimSegId**_](#victimsegid)
    - [_**GCMode**_](#gcmode)
    - [_**AllocateRebuildSegId**_](#allocrebuildsegid)
    - [_**ReleaseRebuildSegId**_](#relrebuildsegid)
    - [_**VictimSegInvPgCnt**_](#victiminvpgcnt)
    - [_**ProhibitUserblkAllocationOnOff**_](#prohibitalloconoff)
  - [**Mapper**](#Mapper)
    - [_**LoadedVolCnt**_](#loadedvolcnt)
    - [_**UnmountedVolId**_](#unmountedvolid)
    - [_**DeletedVolId**_](#deletedvolid)
    - [_**MountedVolCnt**_](#mountedvolcnt)
    - [_**VSAMapLoadPendingIoCnt**_](#vsamaploadpendingiocnt)
    - [_**VSAMapFlushPendingIoCnt**_](#vsamapflushpendingiocnt)
    - [_**StripeMapFlushPendingIoCnt**_](#stripemapflushpendingiocnt)
    - [_**VSAMapFlushedDirtypgCnt**_](#vsamapflusheddirtypgcnt)

---

## **MetaFs**

MetaFs group contains the metrics of the Meta Filsystem.

---

### _**NormalShutdown**_

**ID**: 40000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer}

**Introduced**: v0.10.0

Whether the previous shutdown was normal.

---

### _**PendingMioCount**_

**ID**: 40100

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"thread_name": String}

**Introduced**: v0.10.0

The number of requests requested by user modules per a MetaLpn.

---

### _**PendingMpioCount**_

**ID**: 40101

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"thread_name": String}

**Introduced**: v0.10.0

The Count of requests for follow-up because the event worker has finished processing.

---

## **Volume**

Volume group contains the metrics of volume.

---
### _**read_iops**_

**ID**: 50000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_unique_id": Integer, "volume_id": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The IOPS of read in a volume & an array.

---

### _**read_rate_bytes_per_second**_

**ID**: 50001

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_unique_id": Integer, "volume_id": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The rate(bytes/second) of read in a volume & an array.

---

### _**read_latency_mean_ns**_

**ID**: 50002

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_unique_id": Integer, "volume_id": Integer, "sample_count": Integer, "interval": Integer}

**Introduced**: v0.10.0

The mean value of read latency in a volume & an array.

---

### _**read_latency_max_ns**_

**ID**: 50003

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_unique_id": Integer, "volume_id": Integer, "sample_count": Integer, "interval": Integer}

**Introduced**: v0.10.0

The max value of read latency in a volume & an array.

---

### _**write_iops**_

**ID**: 50010

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_unique_id": Integer, "volume_id": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The IOPS of write in a volume & an array.

---

### _**write_rate_bytes_per_second**_

**ID**: 50011

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_unique_id": Integer, "volume_id": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The rate(bytes/second) of write in a volume & an array.

---

### _**write_latency_mean_ns**_

**ID**: 50012

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_unique_id": Integer, "volume_id": Integer, "sample_count": Integer, "interval": Integer}

**Introduced**: v0.10.0

The mean value of write latency in a volume & an array.

---

### _**write_latency_max_ns**_

**ID**: 50013

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_unique_id": Integer, "volume_id": Integer, "sample_count": Integer, "interval": Integer}

**Introduced**: v0.10.0

The max value of write latency in a volume & an array.

---

## **Array**

Array group contains the metrics of array

---

### _**ArrayStatus**_

**ID**: 60001

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_unique_id", "array_name", "array_id"}

**Introduced**: v0.10.0

The current status of the array

0: Offline, 1: Normal, 2: Degraded, 3: Rebuilding

---

### _**RebuildCount**_

**ID**: 60002

**Type**: Counter

**Monitoring**: Mandatory

**Labels**: {"array_unique_id", "array_name", "array_id"}

**Introduced**: v0.10.0

Total number of rebuilds triggered.

---

### _**GarbageCollectorStatus**_

**ID**: 60003

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_unique_id", "array_name", "array_id"}

**Introduced**: v0.10.0

The current status of garbage collector in the array.

0: None, 1: Normal, 2: Urgent

---
### _**FreeSegmentCount**_

**ID**: 30000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

The max value is the number of segments in array

---
### _**ContextManagerPendingIoCount**_

**ID**: 30001

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

ContextManager's pendingIo count

---
### _**GCVictimSegId**_

**ID**: 30002

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

Current victim segment

---
### _**GCMode**_

**ID**: 30003

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

Current victim segment

---
### _**VictimSegInvPgCnt**_

**ID**: 30010

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

Current victim segment's invalid page count when it was chosen

---
### _**ProhibitUserBlkAllocationOnOff**_

**ID**: 30011

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

Prohibit free block allocation On or Off
Prohibited: 1, Permitted: 0

---
### _**LoadedVolumeCount**_

**ID**: 33002

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

Loaded volume count

---
### _**UnmountedVolumeCount**_

**ID**: 33004

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

Unmounted volume count

---
### _**MountedVolumeCount**_

**ID**: 33006

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

Mounted volume count

---
### _**VSAMapLoadPendingIoCount**_

**ID**: 33007

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

VSAMap Load pendingIo count

---
### _**VSAMapFlushPendingIoCount**_

**ID**: 33008

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

VSAMap Flush pendingIo count

---
### _**StripeMapFlushPendingIoCount**_

**ID**: 33009

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

StripeMap Flush pendingIo count

---
### _**VSAMapFlushedDirtyPageCount**_

**ID**: 33010

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

VSAMap Flushed dirty page count

---



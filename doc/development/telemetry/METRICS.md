- [**MetaFs**](#metafs)
  - [_**NormalShutdown**_](#normalshutdown)
  - [_**UserRequest**_](#userrequest)
  - [_**UserRequestCount**_](#userrequestcount)
  - [_**FreeMioCount**_](#freemiocount)
  - [_**FreeMpioCount**_](#freempiocount)
  - [_**SumOfAllTheTimeSpentByMpio**_](#sumofallthetimespentbympio)
  - [_**SumOfProcessedMpioCount**_](#sumofprocessedmpiocount)
  - [_**SumOfAllTheTimeSpentByMio**_](#sumofallthetimespentbymio)
  - [_**SumOfProcessedMioCount**_](#sumofprocessedmiocount)
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
- [**Meta**](#meta)
  - [_**FreeSegmentCount**_](#freesegmentcount)
  - [_**ContextManagerPendingIoCount**_](#contextmanagerpendingiocount)
  - [_**GCVictimSegId**_](#gcvictimsegid)
  - [_**GCMode**_](#gcmode)
  - [_**VictimSegInvPgCnt**_](#victimseginvpgcnt)
  - [_**ProhibitUserBlkAllocationOnOff**_](#prohibituserblkallocationonoff)
  - [_**LoadedVolumeCount**_](#loadedvolumecount)
  - [_**UnmountedVolumeCount**_](#unmountedvolumecount)
  - [_**MountedVolumeCount**_](#mountedvolumecount)
  - [_**VSAMapLoadPendingIoCount**_](#vsamaploadpendingiocount)
  - [_**VSAMapFlushPendingIoCount**_](#vsamapflushpendingiocount)
  - [_**StripeMapFlushPendingIoCount**_](#stripemapflushpendingiocount)
  - [_**VSAMapFlushedDirtyPageCount**_](#vsamapflusheddirtypagecount)

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

### _**UserRequest**_

**ID**: 40010

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"thread_name" : Integer, "io_type" : String, "array_id": Integer, "fd" : Integer}

**Introduced**: v0.10.0

The byte-sized value of reuqests from user modules.

---

### _**UserRequestCount**_

**ID**: 40011

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"thread_name" : Integer, "io_type" : String, "array_id": Integer, "fd" : Integer}

**Introduced**: v0.10.0

The count of reuqests from user modules.

---

### _**FreeMioCount**_

**ID**: 40102

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_name": String}

**Introduced**: v0.10.0

The number of mio to allocate

---

### _**FreeMpioCount**_

**ID**: 40103

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_name": String}

**Introduced**: v0.10.0

The number of mpio to allocate

---

### _**SumOfAllTheTimeSpentByMpio**_

**ID**: 40104

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"thread_name": String}

**Introduced**: v0.10.0

The Sum of all the time spent by mpio in specific period

---

### _**SumOfProcessedMpioCount**_

**ID**: 40105

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"thread_name": String}

**Introduced**: v0.10.0

The Sum of processed mpio count in specific period

---

### _**SumOfAllTheTimeSpentByMio**_

**ID**: 40106

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"thread_name": String}

**Introduced**: v0.10.0

The Sum of all the time spent by mio in specific period

---

### _**SumOfProcessedMioCount**_

**ID**: 40107

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"thread_name": String}

**Introduced**: v0.10.0

The Sum of processed mio count in specific period

---

## **Volume**

Volume group contains the metrics of volume.

---
### _**read_iops**_

**ID**: 50000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The IOPS of read in a volume & an array.

---

### _**read_rate_bytes_per_second**_

**ID**: 50001

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The rate(bytes/second) of read in a volume & an array.

---

### _**read_latency_mean_ns**_

**ID**: 50002

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "sample_count": Integer, "interval": Integer}

**Introduced**: v0.10.0

The mean value of read latency in a volume & an array.

---

### _**read_latency_max_ns**_

**ID**: 50003

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "sample_count": Integer, "interval": Integer}

**Introduced**: v0.10.0

The max value of read latency in a volume & an array.

---

### _**write_iops**_

**ID**: 50010

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The IOPS of write in a volume & an array.

---

### _**write_rate_bytes_per_second**_

**ID**: 50011

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The rate(bytes/second) of write in a volume & an array.

---

### _**write_latency_mean_ns**_

**ID**: 50012

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "sample_count": Integer, "interval": Integer}

**Introduced**: v0.10.0

The mean value of write latency in a volume & an array.

---

### _**write_latency_max_ns**_

**ID**: 50013

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "sample_count": Integer, "interval": Integer}

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

0: NOT_EXIST, 1:EXIST_NORMAL, 2:EXIST_DEGRADED, 3:BROKEN, 4: TRY_MOUNT, 5:TRY_UNMOUNT, 6: NORMAL, 7:DEGRADED, 8:REBUILD


---
## **Meta**


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

The current status of garbage collector in the array.

0: None, 1: Normal, 2: Urgent

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



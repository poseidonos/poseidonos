- [**Volume**](#volume)
  - [_**ReadIops**_](#readiops)
  - [_**ReadBandwidth**_](#readbandwidth)
  - [_**ReadLatencyMean**_](#readlatencymean)
  - [_**ReadLatencyMax**_](#readlatencymax)
  - [_**WriteIops**_](#writeiops)
  - [_**WriteBandwidth**_](#writebandwidth)
  - [_**WriteLatencyMean**_](#writelatencymean)
  - [_**WriteLatencyMax**_](#writelatencymax)
- [**Array**](#array)
  - [_**ArrayStatus**_](#arraystatus)
  - [_**RebuildCount**_](#rebuildcount)
  - [_**GarbageCollectorStatus**_](#garbagecollectorstatus)


---

## **Volume**

Volume group contains the metrics of volume.

---

### _**ReadIops**_

**ID**: 50000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"volume_id": Integer, "array_id": Integer, "thread_id": Integer, "thread_name": String, "timestamp": Integer, "interval": Integer}

**Introduced**: v0.10.0

The IOPS of read in the volume.

---

### _**ReadBandwidth**_

**ID**: 50001

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"volume_id": Integer, "array_id": Integer, "thread_id": Integer, "thread_name": String, "timestamp": Integer, "interval": Integer}

**Introduced**: v0.10.0

The bandwidth of read I/O in the volume.

---

### _**ReadLatencyMean**_

**ID**: 50002

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"volume_id": Integer, "array_id": Integer, "sample_count": Integer, "timestamp": Integer, "interval": Integer}

**Introduced**: v0.10.0

The mean value of read latency in the volume.

---

### _**ReadLatencyMax**_

**ID**: 50003

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"volume_id": Integer, "array_id": Integer, "sample_count": Integer, "timestamp": Integer, "interval": Integer}

**Introduced**: v0.10.0

The max value of read latency in the volume.

---

### _**WriteIops**_

**ID**: 50010

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"volume_id": Integer, "array_id": Integer, "thread_id": Integer, "thread_name": String, "timestamp": Integer, "interval": Integer}

**Introduced**: v0.10.0

The IOPS of write in the volume.

---

### _**WriteBandwidth**_

**ID**: 50011

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"volume_id": Integer, "array_id": Integer, "thread_id": Integer, "thread_name": String, "timestamp": Integer, "interval": Integer}

**Introduced**: v0.10.0

The bandwidth of write I/O in the volume.

---

### _**WriteLatencyMean**_

**ID**: 50012

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"volume_id": Integer, "array_id": Integer, "sample_count": Integer, "timestamp": Integer, "interval": Integer}

**Introduced**: v0.10.0

The mean value of write latency in the volume.

---

### _**WriteLatencyMax**_

**ID**: 50013

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"volume_id": Integer, "array_id": Integer, "sample_count": Integer, "timestamp": Integer, "interval": Integer}

**Introduced**: v0.10.0

The max value of write latency in the volume.

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

**Type**: Gaugage

**Monitoring**: Mandatory

**Labels**: {"array_unique_id", "array_name", "array_id"}

**Introduced**: v0.10.0

The current status of garbage collector in the array.

0: None, 1: Normal, 2: Urgent

---
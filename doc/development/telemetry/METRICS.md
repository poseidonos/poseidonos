- [- _**write_rate_bytes_per_second_per_port**_](#--write_rate_bytes_per_second_per_port)
- [**Common**](#common)
- [Common group contains the metrics from the overall informaition of PoseidonOS](#common-group-contains-the-metrics-from-the-overall-informaition-of-poseidonos)
  - [_**common_process_uptime_second**_](#common_process_uptime_second)
- [**Device**](#device)
  - [_**read_unknown_iops_per_ssd**_](#read_unknown_iops_per_ssd)
  - [_**read_meta_iops_per_ssd**_](#read_meta_iops_per_ssd)
  - [_**read_gc_iops_per_ssd**_](#read_gc_iops_per_ssd)
  - [_**read_host_iops_per_ssd**_](#read_host_iops_per_ssd)
  - [_**read_flush_iops_per_ssd**_](#read_flush_iops_per_ssd)
  - [_**read_rebuild_iops_per_ssd**_](#read_rebuild_iops_per_ssd)
  - [_**read_unknown_rate_bytes_per_second_per_ssd**_](#read_unknown_rate_bytes_per_second_per_ssd)
  - [_**read_meta_rate_bytes_per_second_per_ssd**_](#read_meta_rate_bytes_per_second_per_ssd)
  - [_**read_gc_rate_bytes_per_second_per_ssd**_](#read_gc_rate_bytes_per_second_per_ssd)
  - [_**read_host_rate_bytes_per_second_per_ssd**_](#read_host_rate_bytes_per_second_per_ssd)
  - [_**read_flush_rate_bytes_per_second_per_ssd**_](#read_flush_rate_bytes_per_second_per_ssd)
  - [_**read_rebuild_rate_bytes_per_second_per_ssd**_](#read_rebuild_rate_bytes_per_second_per_ssd)
  - [_**write_unknown_iops_per_ssd**_](#write_unknown_iops_per_ssd)
  - [_**write_meta_iops_per_ssd**_](#write_meta_iops_per_ssd)
  - [_**write_gc_iops_per_ssd**_](#write_gc_iops_per_ssd)
  - [_**write_host_iops_per_ssd**_](#write_host_iops_per_ssd)
  - [_**write_flush_iops_per_ssd**_](#write_flush_iops_per_ssd)
  - [_**write_rebuild_iops_per_ssd**_](#write_rebuild_iops_per_ssd)
  - [_**write_unknown_rate_bytes_per_second_per_ssd**_](#write_unknown_rate_bytes_per_second_per_ssd)
  - [_**write_meta_rate_bytes_per_second_per_ssd**_](#write_meta_rate_bytes_per_second_per_ssd)
  - [_**write_gc_rate_bytes_per_second_per_ssd**_](#write_gc_rate_bytes_per_second_per_ssd)
  - [_**write_host_rate_bytes_per_second_per_ssd**_](#write_host_rate_bytes_per_second_per_ssd)
  - [_**write_flush_rate_bytes_per_second_per_ssd**_](#write_flush_rate_bytes_per_second_per_ssd)
  - [_**write_rebuild_rate_bytes_per_second_per_ssd**_](#write_rebuild_rate_bytes_per_second_per_ssd)
- [**Journal**](#journal)
  - [_**jrn_checkpoint**_](#jrn_checkpoint)
  - [_**jrn_log_group_reset_cnt**_](#jrn_log_group_reset_cnt)
  - [_**jrn_log_group_reset_done_cnt**_](#jrn_log_group_reset_done_cnt)
  - [_**jrn_load_log_group**_](#jrn_load_log_group)
  - [_**jrn_log_count**_](#jrn_log_count)
  - [_**jrn_log_done_count**_](#jrn_log_done_count)
  - [_**jrn_log_write_time_average**_](#jrn_log_write_time_average)
- [**MetaFs**](#metafs)
  - [_**normal_shutdown_npor**_](#normal_shutdown_npor)
  - [_**user_request**_](#user_request)
  - [_**user_request_cnt**_](#user_request_cnt)
  - [_**user_request_publish_cnt_per_interval**_](#user_request_publish_cnt_per_interval)
  - [_**metafs_scheduler_issued_request_count_to_ssd**_](#metafs_scheduler_issued_request_count_to_ssd)
  - [_**metafs_scheduler_issued_request_count_to_nvram**_](#metafs_scheduler_issued_request_count_to_nvram)
  - [_**metafs_scheduler_issued_request_count_to_journal_ssd**_](#metafs_scheduler_issued_request_count_to_journal_ssd)
  - [_**metafs_worker_issued_request_count_partition**_](#metafs_worker_issued_request_count_partition)
  - [_**metafs_worker_done_request_count_partition**_](#metafs_worker_done_request_count_partition)
  - [_**metafs_worker_issued_request_count_file_type**_](#metafs_worker_issued_request_count_file_type)
  - [_**metafs_worker_done_request_count_file_type**_](#metafs_worker_done_request_count_file_type)
  - [_**free_mio_count**_](#free_mio_count)
  - [_**sampled_mpio_time_spent_all_stages**_](#sampled_mpio_time_spent_all_stages)
  - [_**processed_mpio_count**_](#processed_mpio_count)
  - [_**sampled_mio_time_from_issue_to_complete**_](#sampled_mio_time_from_issue_to_complete)
  - [_**sampled_mio_count**_](#sampled_mio_count)
  - [_**mio_handler_is_working**_](#mio_handler_is_working)
  - [_**free_mpio_count**_](#free_mpio_count)
  - [_**sampled_mio_time_spent_all_stages**_](#sampled_mio_time_spent_all_stages)
  - [_**processed_mio_count**_](#processed_mio_count)
  - [_**sampled_mpio_time_from_write_to_release**_](#sampled_mpio_time_from_write_to_release)
  - [_**sampled_mpio_time_from_push_to_pop**_](#sampled_mpio_time_from_push_to_pop)
  - [_**sampled_mpio_count**_](#sampled_mpio_count)
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
  - [_**ArrayUsageBlockCount**_](#arrayusageblockcount)
  - [_**VolumeUsageBlockCount**_](#volumeusageblockcount)
- [**Volume**](#volume-1)
  - [_**CreateVolumeId**_](#createvolumeid)
  - [_**DeleteVolumeId**_](#deletevolumeid)
  - [_**MountVolumeId**_](#mountvolumeid)
  - [_**UnmountVolumeId**_](#unmountvolumeid)
  - [_**QosUpdateVolumeId**_](#qosupdatevolumeid)
  - [_**RenameVolumeId**_](#renamevolumeid)
- [**Resource**](#resource)
  - [_**AvailableMemorySize**_](#availablememorysize)
- [**Disk**](#disk)
  - [_**softMediaErrorLow**_](#softmediaerrorlow)
  - [_**softMediaErrorHigh**_](#softmediaerrorhigh)
  - [_**powerCycleLow**_](#powercyclelow)
  - [_**powerCycleHigh**_](#powercyclehigh)
  - [_**powerOnHourLow**_](#poweronhourlow)
  - [_**powerOnHourHigh**_](#poweronhourhigh)
  - [_**unsafeShutdownsLow**_](#unsafeshutdownslow)
  - [_**unsafeShutdownsHigh**_](#unsafeshutdownshigh)
  - [_**temperature**_](#temperature)
  - [_**availableSpare**_](#availablespare)
  - [_**availableSpareThreshold**_](#availablesparethreshold)
  - [_**percentageUsed**_](#percentageused)
  - [_**controllerBusyTimeLow**_](#controllerbusytimelow)
  - [_**controllerBusyTimeHigh**_](#controllerbusytimehigh)
  - [_**warningTemperatureTime**_](#warningtemperaturetime)
  - [_**criticalTemperatureTime**_](#criticaltemperaturetime)
- [**Network**](#network)
  - [_**read_iops_per_port**_](#read_iops_per_port)
  - [_**read_rate_bytes_per_second_per_port**_](#read_rate_bytes_per_second_per_port)
  - [_**write_iops_per_port**_](#write_iops_per_port)
  - [_**write_rate_bytes_per_second_per_port**_](#write_rate_bytes_per_second_per_port)
---
## **Common**

Common group contains the metrics from the overall informaition of PoseidonOS
---

### _**common_process_uptime_second**_

**ID**: 05000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"version": String}

**Introduced**: v0.11.0

The seconds since PoseidonOS process started

---

## **Device**

Device group contains the metrics of the Devices (NVMe SSD)

---

### _**read_unknown_iops_per_ssd**_

**ID**: 20000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The IOPS of unknown type read in a specific SSD

---

### _**read_meta_iops_per_ssd**_

**ID**: 20001

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The IOPS of meta type read in a specific SSD

---

### _**read_gc_iops_per_ssd**_

**ID**: 20002

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The IOPS of gc type read in a specific SSD

---

### _**read_host_iops_per_ssd**_

**ID**: 20003

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The IOPS of host type read in a specific SSD

---

### _**read_flush_iops_per_ssd**_

**ID**: 20004

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The IOPS of flush type read in a specific SSD

---

### _**read_rebuild_iops_per_ssd**_

**ID**: 20005

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The IOPS of rebuild type read in a specific SSD

---

### _**read_unknown_rate_bytes_per_second_per_ssd**_

**ID**: 20006

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The rate(bytes/second) of unknown type read in a specific SSD

---

### _**read_meta_rate_bytes_per_second_per_ssd**_

**ID**: 20007

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The rate(bytes/second) of meta type read in a specific SSD

---

### _**read_gc_rate_bytes_per_second_per_ssd**_

**ID**: 20008

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The rate(bytes/second) of gc type read in a specific SSD

---

### _**read_host_rate_bytes_per_second_per_ssd**_

**ID**: 20009

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.10.0

The rate(bytes/second) of host type read in a specific SSD

---

### _**read_flush_rate_bytes_per_second_per_ssd**_

**ID**: 20010

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.10.0

The rate(bytes/second) of flush type read in a specific SSD

---

### _**read_rebuild_rate_bytes_per_second_per_ssd**_

**ID**: 20011

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.10.0

The rate(bytes/second) of host type read in a specific SSD

---

### _**write_unknown_iops_per_ssd**_

**ID**: 20012

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The IOPS of unknown type write in a specific SSD

---

### _**write_meta_iops_per_ssd**_

**ID**: 20013

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The IOPS of meta type write in a specific SSD

---

### _**write_gc_iops_per_ssd**_

**ID**: 20014

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The IOPS of gc type write in a specific SSD

---

### _**write_host_iops_per_ssd**_

**ID**: 20015

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The IOPS of host type write in a specific SSD

---

### _**write_flush_iops_per_ssd**_

**ID**: 20016

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The IOPS of flush type write in a specific SSD

---

### _**write_rebuild_iops_per_ssd**_

**ID**: 20017

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The IOPS of rebuild type write in a specific SSD

---

### _**write_unknown_rate_bytes_per_second_per_ssd**_

**ID**: 20018

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The rate(bytes/second) of unknown type write in a specific SSD

---

### _**write_meta_rate_bytes_per_second_per_ssd**_

**ID**: 20019

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The rate(bytes/second) of meta type write in a specific SSD

---

### _**write_gc_rate_bytes_per_second_per_ssd**_

**ID**: 20020

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The rate(bytes/second) of gc type write in a specific SSD

---

### _**write_host_rate_bytes_per_second_per_ssd**_

**ID**: 20021

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "SSD_id": Integer}

**Introduced**: v0.11.0

The rate(bytes/second) of host type write in a specific SSD

---

### _**write_flush_rate_bytes_per_second_per_ssd**_

**ID**: 20022

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "SSD_id": Integer}

**Introduced**: v0.11.0

The rate(bytes/second) of flush type write in a specific SSD

---

### _**write_rebuild_rate_bytes_per_second_per_ssd**_

**ID**: 20023

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "SSD_id": Integer}

**Introduced**: v0.11.0

The rate(bytes/second) of rebuild type write in a specific SSD

---
## **Journal**

Journal group contains the metrics of the Journal.

---

### _**jrn_checkpoint**_

**ID**: 36001

**Type**: Gauge

**Monitoring**: Mandatory

**Introduced**: v0.11.0

The value will be set when checkpoint is running.

---

### _**jrn_log_group_reset_cnt**_

**ID**: 36002

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"group_id": Integer}

**Introduced**: v0.11.0

The count of reset log group

---

### _**jrn_log_group_reset_done_cnt**_

**ID**: 36003

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"group_id": Integer}

**Introduced**: v0.11.0

The count of log group reset done

---

### _**jrn_load_log_group**_

**ID**: 36004

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"group_id": Integer}

**Introduced**: v0.11.0

Whether a log group is being loaded

---

### _**jrn_log_count**_

**ID**: 36005

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"group_id": Integer}

**Introduced**: v0.11.0

The count of log issued

---

### _**jrn_log_done_count**_

**ID**: 36006

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"group_id": Integer}

**Introduced**: v0.11.0

The count of log done

---

### _**jrn_log_write_time_average**_

**ID**: 36007

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

Average of time spent from issue to done

---
## **MetaFs**

MetaFs group contains the metrics of the Meta Filsystem.

---

### _**normal_shutdown_npor**_

**ID**: 40000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer}

**Introduced**: v0.10.0

Whether the previous shutdown was normal.

---

### _**user_request**_

**ID**: 40010

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"thread_name" : Integer, "io_type" : String, "array_id": Integer, "fd" : Integer}

**Introduced**: v0.10.0

The byte-sized value of reuqests from user modules.

---

### _**user_request_cnt**_

**ID**: 40011

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"thread_name" : Integer, "io_type" : String, "array_id": Integer, "fd" : Integer}

**Introduced**: v0.10.0

The count of reuqests from user modules.

---

### _**user_request_publish_cnt_per_interval**_

**ID**: 40012

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.10.0

The count of publishing metrics of metafs

---

### _**metafs_scheduler_issued_request_count_to_ssd**_

**ID**: 40100

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

The request count that the meta scheduler issues to the ssd periodically

---

### _**metafs_scheduler_issued_request_count_to_nvram**_

**ID**: 40101

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

The request count that the meta scheduler issues to the nvram periodically

---

### _**metafs_scheduler_issued_request_count_to_journal_ssd**_

**ID**: 40102

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

The request count that the meta scheduler issues to the journal ssd periodically

---

### _**metafs_worker_issued_request_count_partition**_

**ID**: 40103

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"type": Integer}

**Introduced**: v0.11.0

The request count that the meta worker issues to a partition periodically

---

### _**metafs_worker_done_request_count_partition**_

**ID**: 40104

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"type": Integer}

**Introduced**: v0.11.0

The done count that the meta worker issues to a partition periodically

---

### _**metafs_worker_issued_request_count_file_type**_

**ID**: 40105

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"type": Integer}

**Introduced**: v0.11.0

The request count that the meta worker issues to a file type periodically

---

### _**metafs_worker_done_request_count_file_type**_

**ID**: 40106

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"type": Integer}

**Introduced**: v0.11.0

The done count that the meta worker issues to a file type periodically

---

### _**free_mio_count**_

**ID**: 40200

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.10.0

The count of mio resource which can be allocated

---

### _**sampled_mpio_time_spent_all_stages**_

**ID**: 40201

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

The time in ms during processing mpio

---

### _**processed_mpio_count**_

**ID**: 40202

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

The count of processed mpio

---

### _**sampled_mio_time_from_issue_to_complete**_

**ID**: 40203

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

The time in ms during processing mpio from issue to complete stage

---

### _**sampled_mio_count**_

**ID**: 40204

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

The count of sampled mio

---

### _**mio_handler_is_working**_

**ID**: 40205

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

timestamp

---

### _**free_mpio_count**_

**ID**: 40300

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.10.0

The count of mpio resource which can be allocated

---

### _**sampled_mio_time_spent_all_stages**_

**ID**: 40301

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

The time in ms during processing mio

---

### _**processed_mio_count**_

**ID**: 40302

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

The count of processed mio

---

### _**sampled_mpio_time_from_write_to_release**_

**ID**: 40303

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

The time in ms during processing mio from issue to complete stage

---

### _**sampled_mpio_time_from_push_to_pop**_

**ID**: 40304

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

The time in ms during processing mio from push to pop stage

---

### _**sampled_mpio_count**_

**ID**: 40305

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {}

**Introduced**: v0.11.0

The count of sampled mpio

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


### _**ArrayUsageBlockCount**_

**ID**: 60002

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

Remain Storage Size(block count) In an Array

---

### _**VolumeUsageBlockCount**_

**ID**: 60003

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id", "volume_id"}

**Introduced**: v0.10.0

Remain Storage Size(block count) In a Volume

---
## **Volume**


---
### _**CreateVolumeId**_

**ID**: 90000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

The created volume id

---
### _**DeleteVolumeId**_

**ID**: 90001

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

The deleted volume id

---
### _**MountVolumeId**_

**ID**: 90002

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

The mounted volume id

---
### _**UnmountVolumeId**_

**ID**: 90003

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

The unmounted volume id

---
### _**QosUpdateVolumeId**_

**ID**: 90004

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

The updated volume id

---
### _**RenameVolumeId**_

**ID**: 90005

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"publisher_name", "array_name", "run_id"}

**Introduced**: v0.10.0

The renamed Volume id

---
## **Resource**

Resource group contains the metrics of the pos resource.

---
### _**AvailableMemorySize**_

**ID**: 100000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"node_name": String}

**Introduced**: v0.10.0

Available memory size

---
## **Disk**

disk group contains the smart metrics.

---
### _**softMediaErrorLow**_

**ID**: 110000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Soft Media Error Low Value

---
### _**softMediaErrorHigh**_

**ID**: 110001

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Soft Media Error High Value

---
### _**powerCycleLow**_

**ID**: 110002

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Power cycle Low Value

---
### _**powerCycleHigh**_

**ID**: 110003

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Power cycle High Value

---
### _**powerOnHourLow**_

**ID**: 110004

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Power On Hour Low Value

---
### _**powerOnHourHigh**_

**ID**: 110005

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Power On Hour High Value

---
### _**unsafeShutdownsLow**_

**ID**: 110006

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Unsafe Shutdowns Low Value

---
### _**unsafeShutdownsHigh**_

**ID**: 110007

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Unsafe Shutdowns High Value

---
### _**temperature**_

**ID**: 110008

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Temperature Value

---
### _**availableSpare**_

**ID**: 110009

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Available Spare Value

---
### _**availableSpareThreshold**_

**ID**: 110010

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Available Spare Threshold Value

---
### _**percentageUsed**_

**ID**: 110011

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Percentage Used Value

---
### _**controllerBusyTimeLow**_


**ID**: 110012

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Controller Busy Time Low Value

---
### _**controllerBusyTimeHigh**_

**ID**: 110013

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Controller Busy Time High Value

---
### _**warningTemperatureTime**_

**ID**: 110014

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Warning Temperature Time Value

---
### _**criticalTemperatureTime**_

**ID**: 110015

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Critical Temperature Time Value

---
## **Network**

Network group contains the metrics from the network related metric of PoseidonOS

---

### _**read_iops_per_port**_

**ID**: 120001

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"port": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The IOPS of read in a port.

---

### _**read_rate_bytes_per_second_per_port**_

**ID**: 120002

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"port": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The rate(bytes/second) of read in a port.

---

### _**write_iops_per_port**_

**ID**: 120011

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"port": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The IOPS of write in a port.

---

### _**write_rate_bytes_per_second_per_port**_

**ID**: 120012

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"port": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The rate(bytes/second) of write in a port.
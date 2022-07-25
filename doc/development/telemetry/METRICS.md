- [**Common**](#common)
  - [_**common_process_uptime_second**_](#common_process_uptime_second)

- [**Device**](#device)
  - [_**read_iops_device**_](#read_iops_device)
  - [_**read_bps_device**_](#read_bps_device)
  - [_**write_iops_device**_](#write_iops_device)
  - [_**write_bps_device**_](#write_bps_device)

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
  - [_**meta_file_create_request**_](#meta_file_create_request)
  - [_**meta_file_open_request**_](#meta_file_open_request)
  - [_**meta_file_close_request**_](#meta_file_close_request)
  - [_**meta_file_delete_request**_](#meta_file_delete_request)
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
  - [_**read_iops_volume**_](#read_iops_volume)
  - [_**read_bps_volume**_](#read_bps_volume)
  - [_**read_avg_lat_volume**_](#read_avg_lat_volume)
  - [_**write_iops_volume**_](#write_iops_volume)
  - [_**write_bps_volume**_](#write_bps_volume)
  - [_**write_avg_lat_volume**_](#write_avg_lat_volume)
  - [_**volume_state**_](#volume_state)
  - [_**volume_total_capacity**_](#volume_total_capacity)

- [**Array**](#array)
  - [_**ArrayStatus**_](#arraystatus)

- [**Network**](#network)
  - [_**read_iops_network**_](#read_iops_network)
  - [_**read_bps_network**_](#read_bps_network)
  - [_**write_iops_network**_](#write_iops_network)
  - [_**write_bps_network**_](#write_bps_network)

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

- [**Resource**](#resource)
  - [_**AvailableMemorySize**_](#availablememorysize)

- [**Disk**](#disk)
  - [_**softMediaErrorLower**_](#softmediaerrorlower)
  - [_**softMediaErrorUpper**_](#softmediaerrorupper)
  - [_**powerCycleLower**_](#powercyclelower)
  - [_**powerCycleUpper**_](#powercyclehupper)
  - [_**powerOnHourLow**_](#poweronhourlower)
  - [_**powerOnHourUpper**_](#poweronhourUpper)
  - [_**unsafeShutdownsLow**_](#unsafeshutdownslower)
  - [_**unsafeShutdownsUpper**_](#unsafeshutdownsupper)
  - [_**temperature**_](#temperature)
  - [_**availableSpare**_](#availablespare)
  - [_**availableSpareThreshold**_](#availablesparethreshold)
  - [_**percentageUsed**_](#percentageused)
  - [_**controllerBusyTimeLower**_](#controllerbusytimelower)
  - [_**controllerBusyTimeUpper**_](#controllerbusytimeupper)
  - [_**warningTemperatureTime**_](#warningtemperaturetime)
  - [_**criticalTemperatureTime**_](#criticaltemperaturetime)
  - [_**lifetimeWaf**_](#lifetimewaf)
  - [_**trailingHourWaf**_](#trailinghourwaf)
  - [_**trimSectorCountLower**_](#trimsectorcountlower)
  - [_**trimSectorCountUpper**_](#trimsectorcountupper)
  - [_**hostWrittenCountLower**_](#hostwrittencountlower)
  - [_**hostWrittenCountUpper**_](#hostwrittencountupper)
  - [_**nandWrittenCountLower**_](#nandwrittencountlower)
  - [_**nandWrittenCountUpper**_](#nandwrittencountupper)
  - [_**thermalThrottleEventCount**_](#thermalthrottleeventcount)
  - [_**highestTemperature**_](#highesttemperature)
  - [_**lowestTemeperature**_](#lowesttemeperature)
  - [_**overTemperatureCount**_](#overtemperaturecount)
  - [_**underTemperatureCount**_](#undertemperaturecount)
- [**IOCount**](#iocount)
  - [_**count_of_volume_io_constructors**_](#count_of_volume_io_constructors)
  - [_**count_of_volume_io_destructors**_](#count_of_volume_io_destructors)
  - [_**count_of_ubio_constructors**_](#count_of_ubio_constructors)
  - [_**count_of_ubio_destructors**_](#count_of_ubio_destructors)
  - [_**submission_count_of_ssd_ios**_](#submission_count_of_ssd_ios)
  - [_**completion_count_of_ssd_ios**_](#completion_count_of_ssd_ios)
  - [_**pushing_count_of_event_queue**_](#pushing_count_of_event_queue)
  - [_**pushing_count_of_worker_common_queue**_](#pushing_count_of_worker_common_queue)
  - [_**popping_count_of_worker_common_queue**_](#popping_count_of_worker_common_queue)
  - [_**count_of_callback_contructors**_](#count_of_callback_contructors)
  - [_**count_of_callback_destructors**_](#count_of_callback_destructors)
  - [_**count_of_event_contructors**_](#count_of_event_contructors)
  - [_**count_of_event_destructors**_](#count_of_event_destructors)
  - [_**submission_count_in_io_worker**_](#submission_count_in_io_worker)
  - [_**completion_count_in_io_worker**_](#completion_count_in_io_worker)
  - [_**count_of_requested_user_read**_](#count_of_requested_user_read)
  - [_**count_of_requested_user_write**_](#count_of_requested_user_write)
  - [_**count_of_requested_user_adminio**_](#count_of_requested_user_adminio)
  - [_**count_of_complete_user_read**_](#count_of_complete_user_read)
  - [_**count_of_complete_user_write**_](#count_of_complete_user_write)
  - [_**count_of_complete_user_adminio**_](#count_of_complete_user_adminio)
  - [_**count_of_user_flush_process**_](#count_of_user_flush_process)
  - [_**count_of_partial_write_process**_](#count_of_partial_write_process)
  - [_**count_of_user_fail_io**_](#count_of_user_fail_io)
  - [_**count_of_user_read_pending_cnt**_](#count_of_user_read_pending_cnt)
  - [_**count_of_user_write_pending_cnt**_](#count_of_user_write_pending_cnt)
  - [_**count_of_internal_io_pending_cnt**_](#count_of_internal_io_pending_cnt)
  - [_**count_of_timeout_io_cnt**_](#count_of_timeout_io_cnt)
  
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

### _**read_iops_device**_

**ID**: 20000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "device_id": Integer, "source": String}

**Introduced**: v0.11.0

The IOs/second of specific source read in a specific device_id

---

### _**read_bps_device**_

**ID**: 20001

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "device_id": Integer, "source": String}

**Introduced**: v0.11.0

The Bytes/second of specific source read in a specific device_id

---

### _**write_iops_device**_

**ID**: 20010

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "device_id": Integer, "source": String}

**Introduced**: v0.11.0

The IOs/second of specific source write in a specific device_id

---

### _**write_bps_device**_

**ID**: 20011

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"thread_id": Integer, "thread_name": String, "interval": Integer, "device_id": Integer, "source": String}

**Introduced**: v0.11.0

The Bytes/second of specific source write in a specific device_id

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

### _**meta_file_create_request**_

**ID**: 40013

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_type": Integer, "file_type": Integer, "result": String}

**Introduced**: v0.10.0

The count of creating meta file

---

### _**meta_file_open_request**_

**ID**: 40014

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_type": Integer, "file_type": Integer, "result": String}

**Introduced**: v0.10.0

The count of openning meta file

---

### _**meta_file_close_request**_

**ID**: 40015

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_type": Integer, "file_type": Integer, "result": String}

**Introduced**: v0.10.0

The count of closing meta file

---

### _**meta_file_delete_request**_

**ID**: 40016

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_type": Integer, "file_type": Integer, "result": String}

**Introduced**: v0.10.0

The count of deleting meta file

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

### _**read_iops_volume**_

**ID**: 50000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The IOs/second of read in a volume & an array.

---

### _**read_bps_volume**_

**ID**: 50001

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The Bytes/second of read in a volume & an array.

---

### _**read_avg_lat_volume**_

**ID**: 50002

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "interval": Integer}

**Introduced**: v0.10.0

The average value of read latency in a volume & an array.

---

### _**write_iops_volume**_

**ID**: 50010

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The IOs/second of write in a volume & an array.

---

### _**write_bps_volume**_

**ID**: 50011

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.10.0

The Bytes/second of write in a volume & an array.

---

### _**write_avg_lat_volume**_

**ID**: 50012

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_id": Integer, "interval": Integer}

**Introduced**: v0.10.0

The average value of write latency in a volume & an array.

---

### _**volume_state**_

**ID**: 50020

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_name": string}

**Introduced**: v0.10.0

The voluem satete. 0 : Unmounted, 1 : Mounted, 2 : Offline

---


### _**volume_total_capacity**_

**ID**: 50021

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"array_id": Integer, "volume_name": string}

**Introduced**: v0.10.0

The value of total capacity of volume.

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

## **Network**

Network group contains the metrics from the network related metric of PoseidonOS

---

### _**read_iops_network**_

**ID**: 70000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"port": String, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.11.0

The IOs/second of read in a port.

---

### _**read_bps_network**_

**ID**: 70001

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"port": String, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.11.0

The Bytes/second of read in a port.

---

### _**write_iops_network**_

**ID**: 70010

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"port": String, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.11.0

The IOs/second of write in a port.

---

### _**write_bps_network**_

**ID**: 70011

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"port": String, "thread_id": Integer, "thread_name": String, "interval": Integer}

**Introduced**: v0.11.0

The Bytes/second of write in a port.

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
### _**softMediaErrorLower**_

**ID**: 110000

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Soft Media Error Lower Value

---
### _**softMediaErrorUpper**_

**ID**: 110001

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Soft Media Error Upper Value

---
### _**powerCycleLower**_

**ID**: 110002

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Power cycle Lower Value

---
### _**powerCycleUpper**_

**ID**: 110003

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Power cycle Upper Value

---
### _**powerOnHourLower**_

**ID**: 110004

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Power On Hour Lower Value

---
### _**powerOnHourUpper**_

**ID**: 110005

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Power On Hour Upper Value

---
### _**unsafeShutdownsLower**_

**ID**: 110006

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Unsafe Shutdowns Lower Value

---
### _**unsafeShutdownsUpper**_

**ID**: 110007

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Unsafe Shutdowns Upper Value

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
### _**controllerBusyTimeLower**_

**ID**: 110012

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Controller Busy Time Lower Value

---
### _**controllerBusyTimeUpper**_

**ID**: 110013

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Controller Busy Time Upper Value

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
### _**lifetimeWaf**_

**ID**: 110020

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Life Time Write Amplification Factor

---
### _**trailingHourWaf**_

**ID**: 110021

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Trailing Hour Write Amplification Factor

---
### _**trimSectorCountLower**_

**ID**: 110022

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Trim Sector Count Lower Value

---
### _**trimSectorCountUpper**_

**ID**: 110023

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Trim Sector Count Upper Value

---
### _**hostWrittenCountLower**_

**ID**: 110024

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Host Written Count Lower Value

---
### _**hostWrittenCountUpper**_

**ID**: 110025

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Host Written Count Upper Value

---
### _**nandWrittenCountLower**_

**ID**: 110026

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Nand Written Count Lower Value

---
### _**nandWrittenCountUpper**_

**ID**: 110027

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Nand Written Count Upper Value

---
### _**thermalThrottleEventCount**_

**ID**: 110028

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Thermal Throttle Event Count Value

---
### _**highestTemperature**_

**ID**: 110029

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Highest Temperature Value

---
### _**lowestTemeperature**_

**ID**: 110030

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Lowest Temperature Value

---
### _**overTemperatureCount**_

**ID**: 110031

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Over Temperature Count Value

---
### _**underTemperatureCount**_

**ID**: 110032

**Type**: Gauge

**Monitoring**: Mandatory

**Labels**: {"nvme_ctrl_id": String}

**Introduced**: v0.10.0

Under Temperature Count Value

---

## **IOCount**

IO Count group contains the metrics for internal count related to IO E2E path

---

### _**count_of_volume_io_constructors**_

**ID**: 130001

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated count of volume IO's constructors

---

### _**count_of_volume_io_destructors**_

**ID**: 130002

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated count of volume IO's destructors

---

### _**count_of_ubio_constructors**_

**ID**: 130003

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated count of ubio's constructors

---

### _**count_of_ubio_destructors**_

**ID**: 130004

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated count of ubio's destructors

---

### _**submission_count_of_ssd_ios**_

**ID**: 130005

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated submission count of ssd's IOs

---

### _**completion_count_of_ssd_ios**_

**ID**: 130006

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of ssd's IOs

---

### _**pushing_count_of_event_queue**_

**ID**: 130007

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated pushing count of each event queue located in event scheduler

---

### _**pushing_count_of_worker_common_queue**_

**ID**: 130008

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated pushing count of worker common queue from event queue

---

### _**popping_count_of_worker_common_queue**_

**ID**: 130009

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated popping count of worker common queue

---

### _**count_of_callback_contructors**_

**ID**: 130010

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated count of callback constructors

---

### _**count_of_callback_detructors**_

**ID**: 130011

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated count of callback destructors

---


### _**count_of_event_contructors**_

**ID**: 130012

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated count of event constructors

---

### _**count_of_event_destructors**_

**ID**: 130013

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated count of event destructors

---

### _**submission_count_in_io_worker**_

**ID**: 130014

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated submission count of io worker

---

### _**completion_count_in_io_worker**_

**ID**: 130015

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

### _**count_of_requested_user_read**_

**ID**: 140000

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

### _**count_of_requested_user_write**_

**ID**: 140001

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

### _**count_of_requested_user_adminio**_

**ID**: 140002

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

### _**count_of_complete_user_read**_

**ID**: 140003

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

### _**count_of_complete_user_write**_

**ID**: 140004

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

### _**count_of_complete_user_adminio**_

**ID**: 140005

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

### _**count_of_user_flush_process**_

**ID**: 140006

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

### _**count_of_partial_write_process**_

**ID**: 140007

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

### _**count_of_user_fail_io**_

**ID**: 140008

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

### _**count_of_user_read_pending_cnt**_

**ID**: 140009

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

### _**count_of_user_write_pending_cnt**_

**ID**: 140010

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

### _**count_of_internal_io_pending_cnt**_

**ID**: 140011

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

### _**count_of_timeout_io_cnt**_

**ID**: 140012

**Type**: Count

**Monitoring**: Mandatory

**Labels**: {"index": Integer, "thread_id": Integer, "thread_name": String}

**Introduced**: v0.11.0

The accumulated completion count of io worker

---

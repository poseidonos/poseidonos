/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include <string>
namespace pos
{
#define MAX_NUM_METRICLIST 1000
#define MAX_NUM_LABEL 100
#define TEL_PUBNAME_LABEL_KEY "publisher_name"
#define TEL_RUNID_LABEL_KEY "run_id"

// ID = "modulecode_itemname_collectiontype(count/history)"

static const std::string TEL05000_COMMON_PROCESS_UPTIME_SECOND = "common_process_uptime_second";

static const std::string TEL20000_READ_UNKNOWN_IOPS_PER_SSD = "read_unknown_iops_per_ssd";
static const std::string TEL20001_READ_META_IOPS_PER_SSD = "read_meta_iops_per_ssd";
static const std::string TEL20002_READ_GC_IOPS_PER_SSD = "read_gc_iops_per_ssd";
static const std::string TEL20003_READ_HOST_IOPS_PER_SSD = "read_host_iops_per_ssd";
static const std::string TEL20004_READ_FLUSH_IOPS_PER_SSD = "read_flush_iops_per_ssd";
static const std::string TEL20005_READ_REBUILD_IOPS_PER_SSD = "read_rebuild_iops_per_ssd";
static const std::string TEL20006_READ_UNKNOWN_RATE_BYTES_PER_SECOND_PER_SSD = "read_unknown_rate_bytes_per_second_per_ssd";
static const std::string TEL20007_READ_META_RATE_BYTES_PER_SECOND_PER_SSD = "read_meta_rate_bytes_per_second_per_ssd";
static const std::string TEL20008_READ_GC_RATE_BYTES_PER_SECOND_PER_SSD = "read_gc_rate_bytes_per_second_per_ssd";
static const std::string TEL20009_READ_HOST_RATE_BYTES_PER_SECOND_PER_SSD = "read_host_rate_bytes_per_second_per_ssd";
static const std::string TEL20010_READ_FLUSH_RATE_BYTES_PER_SECOND_PER_SSD = "read_flush_rate_bytes_per_second_per_ssd";
static const std::string TEL20011_READ_REBUILD_RATE_BYTES_PER_SECOND_PER_SSD = "read_rebuild_rate_bytes_per_second_per_ssd";
static const std::string TEL20012_WRITE_UNKNOWN_IOPS_PER_SSD = "write_unknown_iops_per_ssd";
static const std::string TEL20013_WRITE_META_IOPS_PER_SSD = "write_meta_iops_per_ssd";
static const std::string TEL20014_WRITE_GC_IOPS_PER_SSD = "write_gc_iops_per_ssd";
static const std::string TEL20015_WRITE_HOST_IOPS_PER_SSD = "write_host_iops_per_ssd";
static const std::string TEL20016_WRITE_FLUSH_IOPS_PER_SSD = "write_flush_iops_per_ssd";
static const std::string TEL20017_WRITE_REBUILD_IOPS_PER_SSD = "write_rebuild_iops_per_ssd";
static const std::string TEL20018_WRITE_UNKNOWN_RATE_BYTES_PER_SECOND_PER_SSD = "write_unknown_rate_bytes_per_second_per_ssd";
static const std::string TEL20019_WRITE_META_RATE_BYTES_PER_SECOND_PER_SSD = "write_meta_rate_bytes_per_second_per_ssd";
static const std::string TEL20020_WRITE_GC_RATE_BYTES_PER_SECOND_PER_SSD = "write_gc_rate_bytes_per_second_per_ssd";
static const std::string TEL20021_WRITE_HOST_RATE_BYTES_PER_SECOND_PER_SSD = "write_host_rate_bytes_per_second_per_ssd";
static const std::string TEL20022_WRITE_FLUSH_RATE_BYTES_PER_SECOND_PER_SSD = "write_flush_rate_bytes_per_second_per_ssd";
static const std::string TEL20023_WRITE_REBUILD_RATE_BYTES_PER_SECOND_PER_SSD = "write_rebuild_rate_bytes_per_second_per_ssd";

static const std::string TEL30000_ALCT_FREE_SEG_CNT = "alct_free_seg_cnt";
static const std::string TEL30001_ALCT_ALCTX_PENDINGIO_CNT = "alct_allocctx_pendio_cnt";
static const std::string TEL30002_ALCT_GCVICTIM_SEG = "alct_gcvictim_segid";
static const std::string TEL30003_ALCT_GCMODE = "alct_gcmode";
static const std::string TEL30004_ALCT_RSV = "rsv";
static const std::string TEL30005_ALCT_RSV = "rsv";
static const std::string TEL30006_ALCT_RSV = "rsv";
static const std::string TEL30007_ALCT_RSV = "rsv";
static const std::string TEL30008_ALCT_RSV = "rsv";
static const std::string TEL30009_ALCT_RSV = "rsv";
static const std::string TEL30010_ALCT_VICTIM_SEG_INVALID_PAGE_CNT = "alct_victim_invalidpg_cnt";
static const std::string TEL30011_ALCT_PROHIBIT_USERBLK_ALLOCATION_ONOFF = "alct_prohibit_userblk_alloc_onoff";

static const std::string TEL33000_MAP_RSV = "rsv";
static const std::string TEL33001_MAP_RSV = "rsv";
static const std::string TEL33002_MAP_LOADED_VOL_CNT = "map_loaded_int_volid";
static const std::string TEL33003_MAP_RSV = "rsv";
static const std::string TEL33004_MAP_UNMOUNTED_VOL = "map_unmounted_volid";
static const std::string TEL33005_MAP_RSV = "rsv";
static const std::string TEL33006_MAP_MOUNTED_VOL_CNT = "map_mounted_volume_cnt";
static const std::string TEL33007_MAP_VSA_LOAD_PENDINGIO_CNT = "map_vsa_load_pendio_cnt";
static const std::string TEL33008_MAP_VSA_FLUSH_PENDINGIO_CNT = "map_vsa_flush_pendio_cnt";
static const std::string TEL33009_MAP_STRIPE_FLUSH_PENDINGIO_CNT = "map_stripe_flush_pendio_cnt";
static const std::string TEL33010_MAP_VSA_FLUSHED_DIRTYPAGE_CNT = "map_vsa_flushed_dirtypg_cnt";
static const std::string TEL33011_MAP_REVERSE_FLUSH_IO_ISSUED_CNT = "map_reverse_flush_io_issued_cnt";
static const std::string TEL33012_MAP_REVERSE_FLUSH_IO_DONE_CNT = "map_reverse_flush_io_done_cnt";

static const std::string TEL36000_JRN_ = "j_test";
static const std::string TEL36001_JRN_CHECKPOINT = "jrn_checkpoint";
static const std::string TEL36002_JRN_LOG_GROUP_RESET_CNT = "jrn_log_group_reset_cnt";
static const std::string TEL36003_JRN_LOG_GROUP_RESET_DONE_CNT = "jrn_log_group_reset_done_cnt";
static const std::string TEL36004_JRN_LOAD_LOG_GROUP = "jrn_load_log_group";
static const std::string TEL36005_JRN_LOG_COUNT = "jrn_log_count";
static const std::string TEL36006_JRN_LOG_DONE_COUNT = "jrn_log_done_count";
static const std::string TEL36007_JRN_LOG_WRITE_TIME_AVERAGE = "jrn_log_write_time_average";
static const std::string TEL39999_JRN_ = "j_test_end";

static const std::string TEL40000_METAFS_NORMAL_SHUTDOWN = "normal_shutdown_npor";
static const std::string TEL40010_METAFS_USER_REQUEST = "user_request";
static const std::string TEL40011_METAFS_USER_REQUEST_CNT = "user_request_cnt";
static const std::string TEL40012_METAFS_USER_REQUEST_PUBLISH_CNT_PER_INTERVAL = "user_request_publish_cnt_per_interval";
static const std::string TEL40100_METAFS_SCHEDULER_ISSUE_COUNT_TO_SSD = "metafs_scheduler_issued_request_count_to_ssd";
static const std::string TEL40101_METAFS_SCHEDULER_ISSUE_COUNT_TO_NVRAM = "metafs_scheduler_issued_request_count_to_nvram";
static const std::string TEL40102_METAFS_SCHEDULER_ISSUE_COUNT_TO_JOURNAL_SSD = "metafs_scheduler_issued_request_count_to_journal_ssd";
static const std::string TEL40103_METAFS_WORKER_ISSUE_COUNT_PARTITION = "metafs_worker_issued_request_count_partition";
static const std::string TEL40104_METAFS_WORKER_DONE_COUNT_PARTITION = "metafs_worker_done_request_count_partition";
static const std::string TEL40105_METAFS_WORKER_ISSUE_COUNT_FILE_TYPE = "metafs_worker_issued_request_count_file_type";
static const std::string TEL40106_METAFS_WORKER_DONE_COUNT_FILE_TYPE = "metafs_worker_done_request_count_file_type";
static const std::string TEL40200_METAFS_FREE_MIO_CNT = "free_mio_count";
static const std::string TEL40201_METAFS_MPIO_TIME_SPENT_PROCESSING_ALL_STAGES = "sampled_mpio_time_spent_all_stages";
static const std::string TEL40202_METAFS_PROCESSED_MPIO_COUNT = "processed_mpio_count";
static const std::string TEL40203_METAFS_MIO_TIME_FROM_ISSUE_TO_COMPLETE = "sampled_mio_time_from_issue_to_complete";
static const std::string TEL40204_METAFS_MIO_SAMPLED_COUNT = "sampled_mio_count";
static const std::string TEL40205_METAFS_MIO_HANDLER_IS_WORKING = "mio_handler_is_working";
static const std::string TEL40300_METAFS_FREE_MPIO_CNT = "free_mpio_count";
static const std::string TEL40301_METAFS_MIO_TIME_SPENT_PROCESSING_ALL_STAGES = "sampled_mio_time_spent_all_stages";
static const std::string TEL40302_METAFS_PROCESSED_MIO_COUNT = "processed_mio_count";
static const std::string TEL40303_METAFS_MPIO_TIME_FROM_WRITE_TO_RELEASE = "sampled_mpio_time_from_write_to_release";
static const std::string TEL40304_METAFS_MPIO_TIME_FROM_PUSH_TO_POP = "sampled_mpio_time_from_push_to_pop";
static const std::string TEL40305_METAFS_MPIO_SAMPLED_COUNT = "sampled_mpio_count";

static const std::string TEL50000_READ_IOPS = "read_iops";
static const std::string TEL50001_READ_RATE_BYTES_PER_SECOND = "read_rate_bytes_per_second";
static const std::string TEL50002_READ_LATENCY_MEAN_NS = "read_latency_mean_ns";
static const std::string TEL50003_READ_LATENCY_MAX_NS = "read_latency_max_ns";

static const std::string TEL50010_WRITE_IOPS = "write_iops";
static const std::string TEL50011_WRITE_RATE_BYTES_PER_SECOND = "write_rate_bytes_per_second";
static const std::string TEL50012_WRITE_LATENCY_MEAN_NS = "write_latency_mean_ns";
static const std::string TEL50013_WRITE_LATENCY_MAX_NS = "write_latency_max_ns";

static const std::string TEL60001_ARRAY_STATUS = "array_status";
static const std::string TEL60002_ARRAY_USAGE_BLK_CNT = "array_usage_cnt";
static const std::string TEL60003_VOL_USAGE_BLK_CNT = "volume_usage_cnt";

static const std::string TEL70000_SPDK_REACTOR_UTILIZATION = "spdk_reactor_utilization";

static const std::string TEL80000_DEVICE_PENDING_IO_COUNT = "device_pending_io_count";

static const std::string TEL90000_VOL_CREATE_VOLUME_ID = "create_volume_id";
static const std::string TEL90001_VOL_DELETE_VOLUME_ID = "delete_volume_id";
static const std::string TEL90002_VOL_MOUNT_VOLUME_ID = "mount_volume_id";
static const std::string TEL90003_VOL_UNMOUNT_VOLUME_ID = "unmount_volume_id";
static const std::string TEL90004_VOL_QOS_UPDATE_VOLUME_ID = "update_qos_info_for_volume_id";
static const std::string TEL90005_VOL_RENAME_VOLUME_ID = "rename_volume_id";

static const std::string TEL100000_RESOURCE_CHECKER_AVAILABLE_MEMORY = "available_memory_size";

static const std::string TEL110000_MEDIA_ERROR_COUNT_LOW = "soft_media_error_low";
static const std::string TEL110001_MEDIA_ERROR_COUNT_HIGH = "soft_media_error_high";
static const std::string TEL110002_POWER_CYCLE_LOW = "power_cycle_low";
static const std::string TEL110003_POWER_CYCLE_HIGH = "power_cycle_high";
static const std::string TEL110004_POWER_ON_HOUR_LOW = "power_on_hour_low";
static const std::string TEL110005_POWER_ON_HOUR_HIGH = "power_on_hour_high";
static const std::string TEL110006_UNSAFE_SHUTDOWNS_LOW = "unsafe_shutdowns_low";
static const std::string TEL110007_UNSAFE_SHUTDOWNS_HIGH = "unsafe_shutdowns_high";
static const std::string TEL110008_TEMPERATURE = "temperature";
static const std::string TEL110009_AVAILABLE_SPARE = "available_spare";
static const std::string TEL110010_AVAILABLE_SPARE_THRESHOLD = "available_spare_threshold";
static const std::string TEL110011_PERCENTAGE_USED = "percentage_used";
static const std::string TEL110012_CONTROLLER_BUSY_TIME_LOW = "controller_busy_time_low";
static const std::string TEL110013_CONTROLLER_BUSY_TIME_HIGH = "controller_busy_time_high";
static const std::string TEL110014_WARNING_TEMP_TIME = "warning_temperature_time";
static const std::string TEL110015_CRITICAL_TEMP_TIME = "critical_temperature_time";

static const std::string TEL120001_READ_IOPS_PER_PORT = "read_iops_per_port";
static const std::string TEL120002_READ_RATE_BYTES_PER_SECOND_PER_PORT = "read_rate_bytes_per_second_per_port";
static const std::string TEL120011_WRITE_IOPS_PER_PORT = "write_iops_per_port";
static const std::string TEL120012_WRITE_RATE_BYTES_PER_SECOND_PER_PORT = "write_rate_bytes_per_second_per_port";
} // namespace pos

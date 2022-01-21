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

    static const std::string TEL36000_JRN_ = "j_test";
    static const std::string TEL39999_JRN_ = "j_test_end";

    static const std::string TEL40000_METAFS_NORMAL_SHUTDOWN = "normal_shutdown_npor";
    static const std::string TEL40010_METAFS_USER_REQUEST = "user_request";
    static const std::string TEL40100_METAFS_PENDING_MIO_CNT = "pending_mio_cnt";
    static const std::string TEL40101_METAFS_PENDING_MPIO_CNT = "pending_mpio_cnt";
    static const std::string TEL40102_METAFS_FREE_MIO_CNT = "free_mio_cnt";
    static const std::string TEL40103_METAFS_FREE_MPIO_CNT = "free_mpio_cnt";
    static const std::string TEL40104_METAFS_SUM_OF_ALL_THE_TIME_SPENT_BY_MPIO = "sum_of_all_the_time_spent_by_mpio";
    static const std::string TEL40105_METAFS_SUM_OF_MPIO_COUNT = "sum_of_mpio_count";
    static const std::string TEL40106_METAFS_SUM_OF_ALL_THE_TIME_SPENT_BY_MIO = "sum_of_all_the_time_spent_by_mio";
    static const std::string TEL40107_METAFS_SUM_OF_MIO_COUNT = "sum_of_mio_count";

    static const std::string TEL50000_READ_IOPS = "read_iops";
    static const std::string TEL50001_READ_RATE_BYTES_PER_SECOND = "read_rate_bytes_per_second";
    static const std::string TEL50002_READ_LATENCY_MEAN_NS = "read_latency_mean_ns";
    static const std::string TEL50003_READ_LATENCY_MAX_NS = "read_latency_max_ns";
    static const std::string TEL50010_WRITE_IOPS = "write_iops";
    static const std::string TEL50011_WRITE_RATE_BYTES_PER_SECOND = "write_rate_bytes_per_second";
    static const std::string TEL50012_WRITE_LATENCY_MEAN_NS = "write_latency_mean_ns";
    static const std::string TEL50013_WRITE_LATENCY_MAX_NS = "write_latency_max_ns";

    static const std::string TEL60001_ARRAY_STATUS = "array_status";

} // namespace pos

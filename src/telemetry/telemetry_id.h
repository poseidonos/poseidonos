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

    // ID = "modulecode_itemname_collectiontype(count/history)"
    static const std::string TEL30000_ALCT_FREE_SEG_CNT = "alct_free_seg_cnt";
    static const std::string TEL30001_ALCT_ALCTX_PENDINGIO_CNT = "alct_allocctx_pendio_cnt";
    static const std::string TEL30002_ALCT_GCVICTIM_SEG = "alct_gcvictim_segid";
    static const std::string TEL30003_ALCT_GCMODE = "alct_gcmode";
    static const std::string TEL30004_ALCT_ALLOC_SEG = "alct_alloc_segid";
    static const std::string TEL30005_ALCT_FREED_SEG = "alct_freed_segid";
    static const std::string TEL30006_ALCT_ALLOC_REBUILD_SEG = "alct_alloc_rebuild_segid";
    static const std::string TEL30007_ALCT_RELEASE_REBUILD_SEG = "alct_rel_rebuild_segid";
    static const std::string TEL30008_ALCT_ALL_OCCUIPIED_SEG = "alct_all_occuipied_segid";
    static const std::string TEL30009_ALCT_REBUILD_TARGET_SEG_CNT = "alct_rebuild_tgt_cnt";
    static const std::string TEL30010_ALCT_VICTIM_SEG_INVALID_PAGE_CNT = "alct_victim_invalidpg_cnt";
    static const std::string TEL30011_ALCT_PROHIBIT_USERBLK_ALLOCATION_ONOFF = "alct_prohibit_userblk_alloc_onoff";

    static const std::string TEL33000_MAP_LOADED_EXTERNAL_VOL = "map_loaded_ex_volid";
    static const std::string TEL33001_MAP_LOADED_INTERNAL_VOL = "map_loaded_int_volid";
    static const std::string TEL33002_MAP_LOADED_VOL_CNT = "map_loaded_int_volid";
    static const std::string TEL33003_MAP_MOUNTED_VOL = "map_mounted_volid";
    static const std::string TEL33004_MAP_UNMOUNTED_VOL = "map_unmounted_volid";
    static const std::string TEL33005_MAP_DELETED_VOL = "map_deleted_volid";
    static const std::string TEL33006_MAP_MOUNTED_VOL_CNT = "map_mounted_volume_cnt";
    static const std::string TEL33007_MAP_VSA_LOAD_PENDINGIO_CNT = "map_vsa_load_pendio_cnt";
    static const std::string TEL33008_MAP_VSA_FLUSH_PENDINGIO_CNT = "map_vsa_flush_pendio_cnt";
    static const std::string TEL33009_MAP_STRIPE_FLUSH_PENDINGIO_CNT = "map_stripe_flush_pendio_cnt";
    static const std::string TEL33010_MAP_VSA_FLUSHED_DIRTYPAGE_CNT = "map_vsa_flushed_dirtypg_cnt";

    static const std::string TEL36000_JRN_ = "j_test";
    static const std::string TEL39999_JRN_ = "j_test_end";


    static const std::string TEL50000_VOLUME_IOPS_READ = "volume_iops_read";

} // namespace pos

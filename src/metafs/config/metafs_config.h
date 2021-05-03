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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include <cstdint>
#include "src/array_mgmt/array_mgmt_policy.h"

namespace pos
{
class MetaFsConfig
{
public:
    static const int32_t MAX_META_FILE_NUM_SUPPORT = 2048;
    static const uint32_t TYPICAL_NVRAM_META_FILE_KB_SIZE = 64;
    static const uint32_t MAX_ATOMIC_WRITE_PAGE_CNT = 1;
    static const uint32_t MAX_CONCURRENT_IO_CNT = 1024 * 32; // optimized for 128MB size IO. If requested less than this, user may see performance drop
    static const uint32_t DEFAULT_MAX_CORE_COUNT = 128;
    static const uint32_t DEFAULT_MAX_MPIO_CACHE_COUNT = 32; // MAX_CONCURRENT_IO_CNT has to be greater than this count
    static const uint32_t MAX_ARRAY_CNT = ArrayMgmtPolicy::MAX_ARRAY_CNT;
    static const uint32_t MAX_VOLUME_CNT = 256;
};

class MetaFsIoConfig
{
public:
    static const uint64_t META_PAGE_SIZE_IN_BYTES = 4096;
    static const uint64_t META_PAGE_CONTROL_INFO_SIZE = 64;
    static const uint64_t DEFAULT_META_PAGE_DATA_CHUNK_SIZE = META_PAGE_SIZE_IN_BYTES - META_PAGE_CONTROL_INFO_SIZE;
};

#define RANGE_OVERLAP_CHECK_EN 1
#define MPIO_CACHE_EN 1
#define FAKE_META_FILE_DIR "/tmp/mfs_fake/" // also applied to fake_metafs_management_api.cpp
} // namespace pos

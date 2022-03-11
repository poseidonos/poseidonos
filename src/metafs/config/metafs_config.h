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

#include <cstdint>

#include "src/include/array_mgmt_policy.h"

namespace pos
{
class MetaFsConfig
{
public:
    static const uint32_t MAX_META_FILE_NUM_SUPPORT = 1024;
    static const uint32_t MAX_ARRAY_CNT = ArrayMgmtPolicy::MAX_ARRAY_CNT;
    // 256 volume files per a array, 18 alternative files
    static const uint32_t MAX_VOLUME_CNT = 256 + 18;
    // the number of LPN per extent = 8, sizeof(extent) = 8 * 4K = 32K
    static const uint32_t LPN_COUNT_PER_EXTENT = 8;
    // inode size = 8K = LPN * 2
    static const uint32_t LPN_COUNT_PER_INODE = 2;
    // the number of extents per inode
    static const uint32_t MAX_PAGE_MAP_CNT = 384;
    // char: A55A
    static const uint16_t SIGNATURE_INODE_VERSION = 42330;
    static const uint16_t CURRENT_INODE_VERSION = 1;
    static const size_t MIN_TIME_INTERVAL = 100;
};

class MetaFsIoConfig
{
public:
    // 4K LPN
    static const uint64_t META_PAGE_SIZE_IN_BYTES = 4096;
    // control info size in a LPN
    static const uint64_t META_PAGE_CONTROL_INFO_SIZE = 64;
    // 4032B = 4096B - 64B
    static const uint64_t DEFAULT_META_PAGE_DATA_CHUNK_SIZE = META_PAGE_SIZE_IN_BYTES - META_PAGE_CONTROL_INFO_SIZE;
};
} // namespace pos

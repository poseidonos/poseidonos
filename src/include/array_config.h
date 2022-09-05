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

const uint64_t SIZE_GB = 1UL * 1024 * 1024 * 1024;
const uint64_t SIZE_TB = SIZE_GB * 1024;

namespace pos
{
class ArrayConfig
{
public:
    static const uint32_t NVM_DEVICE_COUNT = 1;
    static const uint64_t MINIMUM_SSD_SIZE_BYTE = 20UL * SIZE_GB;
    static const uint64_t MINIMUM_NVM_SIZE_BYTE = 2UL * SIZE_GB;
    static const uint32_t SECTOR_SIZE_BYTE = 512;
    static const uint32_t BLOCK_SIZE_BYTE = 4096;
    static const uint32_t SECTORS_PER_BLOCK = 8;
    static const uint32_t BLOCKS_PER_CHUNK = 64;
    static const uint64_t STRIPES_PER_SEGMENT = 1024;
    static const uint64_t SSD_SEGMENT_SIZE_BYTE = BLOCK_SIZE_BYTE * BLOCKS_PER_CHUNK * STRIPES_PER_SEGMENT;
    static const uint64_t NVM_MBR_SIZE_BYTE = BLOCK_SIZE_BYTE * BLOCKS_PER_CHUNK;
    static const uint64_t META_NVM_SIZE = 512 * 1024 * 1024; // 512MB

    static const uint64_t MBR_SIZE_BYTE = SSD_SEGMENT_SIZE_BYTE;
    static const uint64_t SSD_PARTITION_START_LBA =
        MBR_SIZE_BYTE / SECTOR_SIZE_BYTE;
    static const uint64_t META_SSD_SIZE_RATIO = 2; // 2% of USER_DATA

    static const uint32_t NVM_SEGMENT_SIZE = 1;
    static const uint32_t JOURNAL_PART_SEGMENT_SIZE = 1;
    static const uint32_t PARITY_COUNT = 1;

    static const uint32_t MIN_WRITE_BLOCK_COUNT = 1;
    static const uint32_t OVER_PROVISIONING_RATIO = 10;
    static const uint32_t REBUILD_STRIPES_UNIT = STRIPES_PER_SEGMENT;
    static const uint32_t REBUILD_CHUNK_SIZE_BYTE = BLOCKS_PER_CHUNK * BLOCK_SIZE_BYTE;
    static const uint32_t MAX_CHUNK_CNT = 32;
};

} // namespace pos

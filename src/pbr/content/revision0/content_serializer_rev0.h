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

#include "src/pbr/content/i_content_serializer.h"
#include "src/pbr/header/header_structure.h"

namespace pbr
{
class ContentSerializerRev0 : public IContentSerializer
{
public:
    ContentSerializerRev0(void) = default;
    virtual ~ContentSerializerRev0(void) = default;
    virtual int Serialize(char* dataOut /* OUT PARAM */, AteData* ateData) override;
    virtual int Deserialize(AteData*& ateOut /* OUT PARAM */, char* rawData) override;
    virtual uint32_t GetContentSize(void) override;
    virtual uint64_t GetContentStartLba(void) override;

private:
    static int _SerializeAte(uint64_t startOffset, char* dataOut /* OUT PARAM */, AteData* ateData);
    static int _DeserializeAte(uint64_t startOffset, AteData* ateOut /* OUT PARAM */, char* rawData);
    static uint64_t _GetBackupAteOffset(void);
    static const uint32_t revision = 0;
    static const uint64_t PBR_CONTENT_START_LBA = header::LENGTH;
    static const uint32_t PBR_CONTENT_SIZE = header::TOTAL_PBR_SIZE - PBR_CONTENT_START_LBA;
    static const uint64_t ATE_START_OFFSET = 0;
    static const uint64_t ATE_SIZE = 24 * 1024;
    static const uint64_t BACKUP_ATE_START_OFFSET = ATE_START_OFFSET + ATE_SIZE;
    static const uint64_t SIGNATURE_OFFSET = 0x0;
    static const uint32_t SIGNATURE_LENGTH = 8;
    static const uint64_t CHECKSUM_OFFSET = 0x8;
    static const uint32_t CHECKSUM_LENGTH = 4;
    static const uint64_t NODE_UUID_OFFSET = 0x10;
    static const uint32_t NODE_UUID_LENGTH = 16;
    static const uint64_t ARRAY_UUID_OFFSET = 0x30;
    static const uint32_t ARRAY_UUID_LENGTH = 16;
    static const uint64_t ARRAY_NAME_OFFSET = 0x50;
    static const uint32_t ARRAY_NAME_LENGTH = 64;
    static const uint64_t CREATED_DT_OFFSET = 0x90;
    static const uint32_t CREATED_DT_LENGTH = 8;
    static const uint64_t LAST_UPDATED_DT_OFFSET = 0x98;
    static const uint32_t LAST_UPDATED_DT_LENGTH = 8;
    static const uint64_t ADE_COUNT_OFFSET = 0xa0;
    static const uint32_t ADE_COUNT_LENGTH = 4;
    static const uint64_t PTE_COUNT_OFFSET = 0xa4;
    static const uint32_t PTE_COUNT_LENGTH = 4;
    static const uint64_t ADE_START_OFFSET = 0xb0;
    static const uint32_t ADE_LENGTH = 128;
    static const uint64_t ADE_SIGNATURE_OFFSET = 0x0;
    static const uint32_t ADE_SIGNATURE_LENGTH = 8;
    static const uint64_t DEV_INDEX_OFFSET = 0x08;
    static const uint32_t DEV_INDEX_LENGTH = 4;
    static const uint64_t DEV_TYPE_OFFSET = 0x18;
    static const uint32_t DEV_TYPE_LENGTH = 4;
    static const uint64_t DEV_STATE_OFFSET = 0x1c;
    static const uint32_t DEV_STATE_LENGTH = 4;
    static const uint64_t DEV_SERIAL_OFFSET = 0x30;
    static const uint32_t DEV_SERIAL_LENGTH = 64;
    static const uint64_t PTE_START_OFFSET = 0x40b0;
    static const uint32_t PTE_LENGTH = 128;
    static const uint64_t PTE_SIGNATURE_OFFSET = 0x0;
    static const uint32_t PTE_SIGNATURE_LENGTH = 8;
    static const uint64_t PART_TYPE_OFFSET = 0x18;
    static const uint32_t PART_TYPE_LENGTH = 4;
    static const uint64_t RAID_TYPE_OFFSET = 0x1c;
    static const uint32_t RAID_TYPE_LENGTH = 4;
    static const uint64_t PART_START_LBA_OFFSET = 0x30;
    static const uint32_t PART_START_LBA_LENGTH = 8;
    static const uint64_t PART_LAST_LBA_OFFSET = 0x38;
    static const uint32_t PART_LAST_LBA_LENGTH = 8;
};

} // namespace pbr

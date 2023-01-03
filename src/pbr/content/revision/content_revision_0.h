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
#include "src/pbr/dto/ate_data.h"
#include "src/pbr/header/header_structure.h"

namespace pbr
{
class ContentRevision_0 : public IContentSerializer
{
public:
    virtual ~ContentRevision_0();

protected:
    virtual int Serialize(char* dataOut, AteData* ateData) override;
    virtual int Deserialize(AteData* out, char* rawData) override;
    virtual uint32_t GetRevision(void) override;
    virtual uint32_t GetContentSize(void) override;
    virtual uint64_t GetContentStartLba(void) override;

private:
    int _SerializeAte(uint64_t startOffset, char* dataOut, AteData* ateData);
    int _DeserializeAte(uint64_t startOffset, AteData* out, char* rawData);
    uint64_t _GetBackupAteOffset(void);
    const uint32_t revision = 0;
    const uint32_t TOTAL_PBR_SIZE = 64 * 1024;
    const uint64_t PBR_CONTENT_START_LBA = pbr::structure::header::LENGTH;
    const uint32_t PBR_CONTENT_SIZE = TOTAL_PBR_SIZE - PBR_CONTENT_START_LBA;

    const uint64_t ATE_START_OFFSET = 0;
    const uint64_t ATE_SIZE = 24 * 1024;
    const uint64_t BACKUP_ATE_START_OFFSET = ATE_START_OFFSET + ATE_SIZE;

    const uint64_t SIGNATURE_OFFSET = 0x0;
    const uint32_t SIGNATURE_LENGTH = 8;
    const uint64_t CHECKSUM_OFFSET = 0x8;
    const uint32_t CHECKSUM_LENGTH = 4;
    const uint64_t NODE_UUID_OFFSET = 0x10;
    const uint32_t NODE_UUID_LENGTH = 32;
    const uint64_t ARRAY_UUID_OFFSET = 0x30;
    const uint32_t ARRAY_UUID_LENGTH = 32;
    const uint64_t ARRAY_NAME_OFFSET = 0x50;
    const uint32_t ARRAY_NAME_LENGTH = 64;
    const uint64_t CREATED_DT_OFFSET = 0x90;
    const uint32_t CREATED_DT_LENGTH = 8;
    const uint64_t LAST_UPDATED_DT_OFFSET = 0x98;
    const uint32_t LAST_UPDATED_DT_LENGTH = 8;

    const uint64_t ADE_COUNT_OFFSET = 0x100;
    const uint32_t ADE_COUNT_LENGTH = 4;
    const uint64_t PTE_COUNT_OFFSET = 0x104;
    const uint32_t PTE_COUNT_LENGTH = 4;

    const uint64_t ADE_START_OFFSET = 0x110;
    const uint32_t ADE_LENGTH = 128;
    const uint64_t ADE_SIGNATURE_OFFSET = 0x0;
    const uint32_t ADE_SIGNATURE_LENGTH = 8;
    const uint64_t DEV_INDEX_OFFSET = 0x08;
    const uint32_t DEV_INDEX_LENGTH = 4;
    const uint64_t DEV_TYPE_GUID_OFFSET = 0x10;
    const uint32_t DEV_TYPE_GUID_LENGTH = 16;
    const uint64_t DEV_STATE_GUID_OFFSET = 0x20;
    const uint32_t DEV_STATE_GUID_LENGTH = 16;
    const uint64_t DEV_SERIAL_GUID_OFFSET = 0x30;
    const uint32_t DEV_SERIAL_GUID_LENGTH = 64;

    const uint64_t PTE_START_OFFSET = 0x4110;
    const uint32_t PTE_LENGTH = 128;
    const uint64_t PTE_SIGNATURE_OFFSET = 0x0;
    const uint32_t PTE_SIGNATURE_LENGTH = 8;
    const uint64_t PART_TYPE_GUID_OFFSET = 0x10;
    const uint32_t PART_TYPE_GUID_LENGTH = 16;
    const uint64_t RAID_TYPE_GUID_OFFSET = 0x20;
    const uint32_t RAID_TYPE_GUID_LENGTH = 16;
    const uint64_t PART_START_LBA_OFFSET = 0x30;
    const uint32_t PART_START_LBA_LENGTH = 8;
    const uint64_t PART_LAST_LBA_OFFSET = 0x38;
    const uint32_t PART_LAST_LBA_LENGTH = 8;
};

} // namespace pbr

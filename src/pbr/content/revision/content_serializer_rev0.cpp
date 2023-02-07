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

#include "content_serializer_rev0.h"
#include "src/pbr/checker/pbr_checksum.h"
#include "src/helper/string/hex_string_converter.h"
#include "src/node/node_info.h"
#include "src/helper/uuid/uuid_helper.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

#include <string.h>

namespace pbr
{

int
ContentSerializerRev0::Serialize(char* dataOut, AteData* ateData)
{
    int ret = _SerializeAte(ATE_START_OFFSET, dataOut, ateData);
    if (ret == 0)
    {
        uint64_t backupAteOffset = _GetBackupAteOffset();
        ret = _SerializeAte(backupAteOffset, dataOut, ateData);
    }
    return ret;
}

int
ContentSerializerRev0::Deserialize(AteData* ateOut, char* rawData)
{
    int ret = _DeserializeAte(ATE_START_OFFSET, ateOut, rawData);
    if (ret != 0)
    {
        uint64_t backupAteOffset = _GetBackupAteOffset();
        ret = _DeserializeAte(backupAteOffset, ateOut, rawData);
    }
    return ret;
}

uint32_t
ContentSerializerRev0::GetContentSize(void)
{
    return PBR_CONTENT_SIZE;
}

uint64_t
ContentSerializerRev0::GetContentStartLba(void)
{
    return PBR_CONTENT_START_LBA;
}

int
ContentSerializerRev0::_SerializeAte(uint64_t startOffset, char* dataOut, AteData* ateData)
{
    memset(&dataOut[startOffset], 0, ATE_SIZE);
    strncpy(&dataOut[startOffset + SIGNATURE_OFFSET], ATE_SIGNATURE.c_str(), SIGNATURE_LENGTH);
    UuidHelper::UuidToByte(pos::NodeInfo::GetUuid(), &dataOut[startOffset + NODE_UUID_OFFSET]);
    UuidHelper::UuidToByte(ateData->arrayUuid, &dataOut[startOffset + ARRAY_UUID_OFFSET]);
    strncpy(&dataOut[startOffset + ARRAY_NAME_OFFSET], ateData->arrayName.c_str(), ARRAY_NAME_LENGTH);
    uint64_to_hex(ateData->createdDateTime, &dataOut[startOffset + CREATED_DT_OFFSET], CREATED_DT_LENGTH);
    uint64_to_hex(ateData->lastUpdatedDateTime, &dataOut[startOffset + LAST_UPDATED_DT_OFFSET], LAST_UPDATED_DT_LENGTH);
    uint32_to_hex(ateData->adeList.size(), &dataOut[startOffset + ADE_COUNT_OFFSET], ADE_COUNT_LENGTH);
    uint32_to_hex(ateData->pteList.size(), &dataOut[startOffset + PTE_COUNT_OFFSET], PTE_COUNT_LENGTH);

    uint64_t adeOffset = startOffset + ADE_START_OFFSET;
    for (AdeData* ade : ateData->adeList)
    {
        strncpy(&dataOut[adeOffset + ADE_SIGNATURE_OFFSET], ADE_SIGNATURE.c_str(), ADE_SIGNATURE_LENGTH);
        uint32_to_hex(ade->devIndex, &dataOut[adeOffset + DEV_INDEX_OFFSET], DEV_INDEX_LENGTH);
        uint32_to_hex(ade->devType, &dataOut[adeOffset + DEV_TYPE_OFFSET], DEV_TYPE_LENGTH);
        uint32_to_hex(ade->devState, &dataOut[adeOffset + DEV_STATE_OFFSET], DEV_STATE_LENGTH);
        strncpy(&dataOut[adeOffset + DEV_SERIAL_OFFSET], ade->devSn.c_str(), DEV_SERIAL_LENGTH);
        adeOffset += ADE_LENGTH;
    }

    uint64_t pteOffset = startOffset + PTE_START_OFFSET;
    for (PteData* pte : ateData->pteList)
    {
        strncpy(&dataOut[pteOffset + PTE_SIGNATURE_OFFSET], PTE_SIGNATURE.c_str(), PTE_SIGNATURE_LENGTH);
        uint32_to_hex(pte->partType, &dataOut[pteOffset + PART_TYPE_OFFSET], PART_TYPE_LENGTH);
        uint32_to_hex(pte->raidType, &dataOut[pteOffset + RAID_TYPE_OFFSET], RAID_TYPE_LENGTH);
        uint64_to_hex(pte->startLba, &dataOut[pteOffset + PART_START_LBA_OFFSET], PART_START_LBA_LENGTH);
        uint64_to_hex(pte->lastLba, &dataOut[pteOffset + PART_LAST_LBA_OFFSET], PART_LAST_LBA_LENGTH);
        pteOffset += PTE_LENGTH;
    }

    uint32_t checksum = MakePbrChecksum(&dataOut[startOffset], ATE_SIZE, CHECKSUM_OFFSET, CHECKSUM_LENGTH);
    uint32_to_hex(checksum, &dataOut[startOffset + CHECKSUM_OFFSET], CHECKSUM_LENGTH);
    return 0;
}

int ContentSerializerRev0::_DeserializeAte(uint64_t startOffset, AteData* ateOut, char* rawData)
{
    { // ATE signature verification
        char signature[SIGNATURE_LENGTH + 1] = {'\0',};
        strncpy(signature, &rawData[startOffset + SIGNATURE_OFFSET], SIGNATURE_LENGTH);
        if (signature != ATE_SIGNATURE)
        {
            int eid = EID(ATE_UNKNOWN_SIGNATURE);
            POS_TRACE_WARN(eid, "{}", signature);
            return eid;
        }
    } ////
    { // checksum verification
        uint32_t actual = hex_to_uint32(&rawData[startOffset + CHECKSUM_OFFSET], CHECKSUM_LENGTH);
        uint32_t expected = MakePbrChecksum(&rawData[startOffset], ATE_SIZE, CHECKSUM_OFFSET, CHECKSUM_LENGTH);
        if (actual != expected)
        {
            int eid = EID(PBR_CHECKSUM_INVALID);
            POS_TRACE_WARN(eid, "actual_value:{}, expected_value:{}", actual, expected);
            return eid;
        }
        else
        {
            POS_TRACE_DEBUG(EID(PBR_CHECKSUM_VALID), "value:{}", actual);
        }
    } ////
    ateOut->nodeUuid = UuidHelper::UuidFromByte(&rawData[startOffset + NODE_UUID_OFFSET]);
    ateOut->arrayUuid = UuidHelper::UuidFromByte(&rawData[startOffset + ARRAY_UUID_OFFSET]);
    char arrayName[ARRAY_NAME_LENGTH + 1] = {'\0',};
    strncpy(arrayName, &rawData[startOffset + ARRAY_NAME_OFFSET], ARRAY_NAME_LENGTH);
    ateOut->arrayName = arrayName;
    ateOut->createdDateTime = hex_to_uint64(&rawData[startOffset + CREATED_DT_OFFSET], CREATED_DT_LENGTH);
    ateOut->lastUpdatedDateTime = hex_to_uint64(&rawData[startOffset + LAST_UPDATED_DT_OFFSET], LAST_UPDATED_DT_LENGTH);
    uint32_t adeCnt = hex_to_uint32(&rawData[startOffset + ADE_COUNT_OFFSET], ADE_COUNT_LENGTH);
    uint64_t adeOffset = startOffset + ADE_START_OFFSET;
    for (uint32_t i = 0; i < adeCnt; i++)
    {
        AdeData* ade = new AdeData();
        { // ADE signature verification
            char signature[ADE_SIGNATURE_LENGTH + 1] = {'\0',};
            strncpy(signature, &rawData[adeOffset + ADE_SIGNATURE_OFFSET], ADE_SIGNATURE_LENGTH);
            if (signature != ADE_SIGNATURE)
            {
                delete ade;
                int eid = EID(ADE_UNKNOWN_SIGNATURE);
                POS_TRACE_WARN(eid, "{}", signature);
                return eid;
            }
        } ////
            ade->devIndex = hex_to_uint32(&rawData[adeOffset + DEV_INDEX_OFFSET], DEV_INDEX_LENGTH);
            ade->devType = hex_to_uint32(&rawData[adeOffset + DEV_TYPE_OFFSET], DEV_TYPE_LENGTH);;
            ade->devState = hex_to_uint32(&rawData[adeOffset + DEV_STATE_OFFSET], DEV_STATE_LENGTH);
        {
            char devSn[DEV_SERIAL_LENGTH + 1] = {'\0',};
            strncpy(devSn, &rawData[adeOffset + DEV_SERIAL_OFFSET], DEV_SERIAL_LENGTH);
            ade->devSn = devSn;
        }
        ateOut->adeList.push_back(ade);
        adeOffset += ADE_LENGTH;
    }
    uint32_t pteCnt = hex_to_uint32( &rawData[startOffset + PTE_COUNT_OFFSET], PTE_COUNT_LENGTH);
    uint64_t pteOffset = startOffset + PTE_START_OFFSET;
    for (uint32_t i = 0; i < pteCnt; i++)
    {
        PteData* pte = new PteData();
        { // ADE signature verification
            char signature[PTE_SIGNATURE_LENGTH + 1] = {'\0',};
            strncpy(signature, &rawData[pteOffset + PTE_SIGNATURE_OFFSET], PTE_SIGNATURE_LENGTH);
            if (signature != PTE_SIGNATURE)
            {
                delete pte;
                int eid = EID(PTE_UNKNOWN_SIGNATURE);
                POS_TRACE_WARN(eid, "{}", signature);
                return eid;
            }
        } ////
        pte->partType = hex_to_uint32(&rawData[pteOffset + PART_TYPE_OFFSET], PART_TYPE_LENGTH);
        pte->raidType = hex_to_uint32(&rawData[pteOffset + RAID_TYPE_OFFSET], RAID_TYPE_LENGTH);
        pte->startLba = hex_to_uint64(&rawData[pteOffset + PART_START_LBA_OFFSET], PART_START_LBA_LENGTH);
        pte->lastLba = hex_to_uint64(&rawData[pteOffset + PART_LAST_LBA_OFFSET], PART_LAST_LBA_LENGTH);
        ateOut->pteList.push_back(pte);
        pteOffset += PTE_LENGTH;
    }
    return 0;
}

uint64_t
ContentSerializerRev0::_GetBackupAteOffset(void)
{
    return ATE_START_OFFSET + ATE_SIZE;
}
} // namespace pbr

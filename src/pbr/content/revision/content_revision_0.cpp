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

#include "content_revision_0.h"
#include "src/pbr/helper/hex_string_converter.h"

#include <string.h>

namespace pbr
{
ContentRevision_0::~ContentRevision_0()
{
}

int
ContentRevision_0::Serialize(char* dataOut, AteData* ateData)
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
ContentRevision_0::Deserialize(AteData* out, char* rawData)
{
    int ret = _DeserializeAte(ATE_START_OFFSET, out, rawData);
    if (ret != 0)
    {
        uint64_t backupAteOffset = _GetBackupAteOffset();
        ret = _DeserializeAte(backupAteOffset, out, rawData);
    }
    return ret;
}

uint32_t
ContentRevision_0::GetRevision(void)
{
    return revision;
}

uint32_t
ContentRevision_0::GetContentSize(void)
{
    return PBR_CONTENT_SIZE;
}

uint64_t
ContentRevision_0::GetContentStartLba(void)
{
    return PBR_CONTENT_START_LBA;
}

int
ContentRevision_0::_SerializeAte(uint64_t startOffset, char* dataOut, AteData* ateData)
{
    {
        uint64_t baseOffset = startOffset;
        {
            char* signature = &dataOut[baseOffset + SIGNATURE_OFFSET];
            strncpy(signature, ateData->signature.c_str(), SIGNATURE_LENGTH);
        }
        {
            string tmp = uint32_to_hexString(ateData->checksum);
            char* checksum = &dataOut[baseOffset + CHECKSUM_OFFSET];
            strncpy(checksum, tmp.c_str(), CHECKSUM_LENGTH);
        }
        {
            char* nodeUuid = &dataOut[baseOffset + NODE_UUID_OFFSET];
            strncpy(nodeUuid, ateData->nodeUuid.c_str(), NODE_UUID_LENGTH);
        }
        {
            char* arrayUuid = &dataOut[baseOffset + ARRAY_UUID_OFFSET];
            strncpy(arrayUuid, ateData->arrayUuid.c_str(), ARRAY_UUID_LENGTH);
        }
        {
            char* arrayName = &dataOut[baseOffset + ARRAY_NAME_OFFSET];
            strncpy(arrayName, ateData->arrayName.c_str(), ARRAY_NAME_LENGTH);
        }
        {
            string tmp = uint64_to_hexString(ateData->createdDateTime);
            char* createdDt = &dataOut[baseOffset + CREATED_DT_OFFSET];
            strncpy(createdDt, tmp.c_str(), CREATED_DT_LENGTH);
        }
        {
            string tmp = uint64_to_hexString(ateData->lastUpdatedDateTime);
            char* luDt = &dataOut[baseOffset + LAST_UPDATED_DT_OFFSET];
            strncpy(luDt, tmp.c_str(), LAST_UPDATED_DT_LENGTH);
        }
        {
            string tmp = uint32_to_hexString(ateData->adeList.size());
            char* adeCount = &dataOut[baseOffset + ADE_COUNT_OFFSET];
            strncpy(adeCount, tmp.c_str(), ADE_COUNT_LENGTH);
        }
        {
            string tmp = uint32_to_hexString(ateData->pteList.size());
            char* pteCount = &dataOut[baseOffset + PTE_COUNT_OFFSET];
            strncpy(pteCount, tmp.c_str(), PTE_COUNT_LENGTH);
        }
    }
    {
        uint64_t baseOffset = startOffset + ADE_START_OFFSET;
        for (AdeData* ade : ateData->adeList)
        {
            {
                char* signature = &dataOut[baseOffset + ADE_SIGNATURE_OFFSET];
                strncpy(signature, ade->signature.c_str(), ADE_SIGNATURE_LENGTH);
            }
            {
                string tmp = uint32_to_hexString(ade->devIndex);
                char* devIndex = &dataOut[baseOffset + DEV_INDEX_OFFSET];
                strncpy(devIndex, tmp.c_str(), DEV_INDEX_LENGTH);
            }
            {
                char* devType = &dataOut[baseOffset + DEV_TYPE_GUID_OFFSET];
                strncpy(devType, ade->devTypeGuid.c_str(), DEV_TYPE_GUID_LENGTH);
            }
            {
                char* devState = &dataOut[baseOffset + DEV_STATE_GUID_OFFSET];
                strncpy(devState, ade->devStateGuid.c_str(), DEV_STATE_GUID_LENGTH);
            }
            {
                char* devSn = &dataOut[baseOffset + DEV_SERIAL_GUID_OFFSET];
                strncpy(devSn, ade->devSn.c_str(), DEV_SERIAL_GUID_LENGTH);
            }
            baseOffset += ADE_LENGTH;
        }
    }
    {
        uint64_t baseOffset = startOffset + PTE_START_OFFSET;
        for (PteData* pte : ateData->pteList)
        {
            {
                char* signature = &dataOut[baseOffset + PTE_SIGNATURE_OFFSET];
                strncpy(signature, pte->signature.c_str(), PTE_SIGNATURE_LENGTH);
            }
            {
                char* partType = &dataOut[baseOffset + PART_TYPE_GUID_OFFSET];
                strncpy(partType, pte->partTypeGuid.c_str(), PART_TYPE_GUID_LENGTH);
            }
            {
                char* raidType = &dataOut[baseOffset + RAID_TYPE_GUID_OFFSET];
                strncpy(raidType, pte->raidTypeGuid.c_str(), RAID_TYPE_GUID_LENGTH);
            }
            {
                string tmp = uint64_to_hexString(pte->startLba);
                char* startLba = &dataOut[baseOffset + PART_START_LBA_OFFSET];
                strncpy(startLba, tmp.c_str(), PART_START_LBA_LENGTH);
            }
            {
                string tmp = uint64_to_hexString(pte->lastLba);
                char* lastLba = &dataOut[baseOffset + PART_LAST_LBA_OFFSET];
                strncpy(lastLba, tmp.c_str(), PART_LAST_LBA_LENGTH);
            }
            baseOffset += PTE_LENGTH;
        }
    }
    return 0;
}

int ContentRevision_0::_DeserializeAte(uint64_t startOffset, AteData* out, char* rawData)
{
    {
        uint64_t baseOffset = startOffset;
        {
            char signature[SIGNATURE_LENGTH] = {'\0',};
            strncpy(signature, &rawData[baseOffset + SIGNATURE_OFFSET], SIGNATURE_LENGTH);
            out->signature = signature;
        }
        {
            char checksum[CHECKSUM_LENGTH] = {'\0',};
            strncpy(checksum, &rawData[baseOffset + CHECKSUM_OFFSET], CHECKSUM_LENGTH);
            out->checksum = hexString_to_uint32(checksum);
        }
        {
            char nodeUuid[NODE_UUID_LENGTH] = {'\0',};
            strncpy(nodeUuid, &rawData[baseOffset + NODE_UUID_OFFSET], NODE_UUID_LENGTH);
            out->nodeUuid = nodeUuid;
        }
        {
            char arrayUuid[ARRAY_UUID_LENGTH] = {'\0',};
            strncpy(arrayUuid, &rawData[baseOffset + ARRAY_UUID_OFFSET], ARRAY_UUID_LENGTH);
            out->arrayUuid = arrayUuid;
        }
        {
            char arrayName[ARRAY_NAME_LENGTH] = {'\0',};
            strncpy(arrayName, &rawData[baseOffset + ARRAY_NAME_OFFSET], ARRAY_NAME_LENGTH);
            out->arrayName = arrayName;
        }
        {
            char createdDt[CREATED_DT_LENGTH] = {'\0',};
            strncpy(createdDt, &rawData[baseOffset + CREATED_DT_OFFSET], CREATED_DT_LENGTH);
            out->createdDateTime = hexString_to_uint64(createdDt);
        }
        {
            char luDt[LAST_UPDATED_DT_LENGTH] = {'\0',};
            strncpy(luDt, &rawData[baseOffset + LAST_UPDATED_DT_OFFSET], LAST_UPDATED_DT_LENGTH);
            out->lastUpdatedDateTime = hexString_to_uint64(luDt);
        }
        {
            char adeCount[ADE_COUNT_LENGTH] = {'\0',};
            strncpy(adeCount, &rawData[baseOffset + ADE_COUNT_OFFSET], ADE_COUNT_LENGTH);
            uint32_t cnt = hexString_to_uint32(adeCount);
            uint64_t baseOffset = startOffset + ADE_START_OFFSET;
            for (uint32_t i = 0; i < cnt; i++)
            {
                AdeData* ade = new AdeData();
                {
                    char signature[ADE_SIGNATURE_LENGTH] = {'\0',};
                    strncpy(signature, &rawData[baseOffset + ADE_SIGNATURE_OFFSET], ADE_SIGNATURE_LENGTH);
                    ade->signature = signature;
                }
                {
                    char devIndex[DEV_INDEX_LENGTH] = {'\0',};
                    strncpy(devIndex, &rawData[baseOffset + DEV_INDEX_OFFSET], DEV_INDEX_LENGTH);
                    ade->devIndex = hexString_to_uint64(devIndex);
                }
                {
                    char devType[DEV_TYPE_GUID_LENGTH] = {'\0',};
                    strncpy(devType, &rawData[baseOffset + DEV_TYPE_GUID_OFFSET], DEV_TYPE_GUID_LENGTH);
                    ade->devTypeGuid = devType;
                }
                {
                    char devState[DEV_STATE_GUID_LENGTH] = {'\0',};
                    strncpy(devState, &rawData[baseOffset + DEV_STATE_GUID_OFFSET], DEV_STATE_GUID_LENGTH);
                    ade->devStateGuid = devState;
                }
                {
                    char devSn[DEV_SERIAL_GUID_LENGTH] = {'\0',};
                    strncpy(devSn, &rawData[baseOffset + DEV_SERIAL_GUID_OFFSET], DEV_SERIAL_GUID_LENGTH);
                    ade->devSn = devSn;
                }
                out->adeList.push_back(ade);
                baseOffset += ADE_LENGTH;
            }
        }
        {
            char pteCount[PTE_COUNT_LENGTH] = {'\0',};
            strncpy(pteCount, &rawData[baseOffset + PTE_COUNT_OFFSET], PTE_COUNT_LENGTH);
            uint32_t cnt = hexString_to_uint32(pteCount);
            uint64_t baseOffset = startOffset + PTE_START_OFFSET;
            for (uint32_t i = 0; i < cnt; i++)
            {
                PteData* pte = new PteData();
                {
                    char signature[PTE_SIGNATURE_LENGTH] = {'\0',};
                    strncpy(signature, &rawData[baseOffset + PTE_SIGNATURE_OFFSET], PTE_SIGNATURE_LENGTH);
                    pte->signature = signature;
                }
                {
                    char partType[PART_TYPE_GUID_LENGTH] = {'\0',};
                    strncpy(partType, &rawData[baseOffset + PART_TYPE_GUID_OFFSET], PART_TYPE_GUID_LENGTH);
                    pte->partTypeGuid = partType;
                }
                {
                    char raidType[RAID_TYPE_GUID_LENGTH] = {'\0',};
                    strncpy(raidType, &rawData[baseOffset + RAID_TYPE_GUID_OFFSET], RAID_TYPE_GUID_LENGTH);
                    pte->raidTypeGuid = raidType;
                }
                {
                    char startLba[PART_START_LBA_LENGTH] = {'\0',};
                    strncpy(startLba, &rawData[baseOffset + PART_START_LBA_OFFSET], PART_START_LBA_LENGTH);
                    pte->startLba = hexString_to_uint64(startLba);
                }
                {
                    char lastLba[PART_LAST_LBA_LENGTH] = {'\0',};
                    strncpy(lastLba, &rawData[baseOffset + PART_LAST_LBA_OFFSET], PART_LAST_LBA_LENGTH);
                    pte->lastLba = hexString_to_uint64(lastLba);
                }
                out->pteList.push_back(pte);
                baseOffset += PTE_LENGTH;
            }
        }
    }
    return 0;
}

uint64_t
ContentRevision_0::_GetBackupAteOffset(void)
{
    return ATE_START_OFFSET + ATE_SIZE;
}
} // namespace pbr

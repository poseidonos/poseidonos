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

#include "src/mapper/include/mapper_const.h"
#include "src/mapper/vsamap/vsamap_api.h"

namespace pos
{

VSAMapAPI::VSAMapAPI(IVSAMapInternal* vsaMapInt, MapperAddressInfo* info)
: iVSAMapInternal(vsaMapInt),
  addrInfo(info)
{
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        vsaMaps[volumeId] = nullptr;
        isVsaMapAccessable[volumeId] = false;
        isVsaMapInternalAccessable[volumeId] = true;
    }
}

VSAMapAPI::~VSAMapAPI(void)
{
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        if (vsaMaps[volumeId] != nullptr)
        {
            delete vsaMaps[volumeId];
        }
    }
}

VSAMapContent*&
VSAMapAPI::GetVSAMapContent(int volID)
{
    return vsaMaps[volID];
}

bool
VSAMapAPI::_IsVsaMapAccessible(int volID)
{
    return isVsaMapAccessable[volID];
}

int
VSAMapAPI::GetVSAs(int volumeId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray)
{
    if (false == _IsVsaMapAccessible(volumeId))
    {
        POS_TRACE_WARN(EID(VSAMAP_NOT_ACCESSIBLE), "VolumeId:{} is not accessible, maybe unmounted", volumeId);
        for (uint32_t blkIdx = 0; blkIdx < numBlks; ++blkIdx)
        {
            vsaArray[blkIdx] = UNMAP_VSA;
        }
        return -EID(VSAMAP_NOT_ACCESSIBLE);
    }

    VSAMapContent* vsaMap = vsaMaps[volumeId];
    for (uint32_t blkIdx = 0; blkIdx < numBlks; ++blkIdx)
    {
        BlkAddr targetRba = startRba + blkIdx;
        vsaArray[blkIdx] = vsaMap->GetEntry(targetRba);
    }
    return 0;
}

int
VSAMapAPI::SetVSAs(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    if (false == _IsVsaMapAccessible(volumeId))
    {
        POS_TRACE_WARN(EID(VSAMAP_NOT_ACCESSIBLE), "VolumeId:{} is not accessible, maybe unmounted", volumeId);
        return -EID(VSAMAP_NOT_ACCESSIBLE);
    }
    return _UpdateVsaMap(volumeId, startRba, virtualBlks);
}

VirtualBlkAddr
VSAMapAPI::GetVSAInternal(int volumeId, BlkAddr rba, int& caller)
{
    int ret = iVSAMapInternal->EnableInternalAccess(volumeId, caller);
    if (CALLER_EVENT == caller)
    {
        if (isVsaMapInternalAccessable[volumeId] == false)
        {
            caller = NEED_RETRY;
            return UNMAP_VSA;
        }

        if (_IsVSAMapLoaded(volumeId) == false)
        {
            // [Exist Volume Case]
            // 0: The First Internal-Approach
            // -EID(MAP_LOAD_ONGOING): Subsequent Internal-Approaches
            if (0 == ret || -EID(MAP_LOAD_ONGOING) == ret)
            {
                caller = NEED_RETRY;
            }
            // [Deleted Volume Case]
            else if (-EID(VSAMAP_LOAD_FAILURE) == ret)
            {
                // Do nothing
            }
            return UNMAP_VSA;
        }
    }
    else if (ret < 0)
    {
        return UNMAP_VSA;
    }

    caller = OK_READY;
    return _ReadVSA(volumeId, rba);
}

int
VSAMapAPI::SetVSAsInternal(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    int caller = CALLER_NOT_EVENT;
    int ret = iVSAMapInternal->EnableInternalAccess(volumeId, caller);
    if (ret < 0)
    {
        return ret;
    }
    return _UpdateVsaMap(volumeId, startRba, virtualBlks);
}

VirtualBlkAddr
VSAMapAPI::GetRandomVSA(BlkAddr rba)
{
    VirtualBlkAddr vsa;
    vsa.stripeId = rba / addrInfo->blksPerStripe;
    vsa.offset = rba % addrInfo->blksPerStripe;
    return vsa;
}

MpageList
VSAMapAPI::GetDirtyVsaMapPages(int volumeId, BlkAddr startRba, uint64_t numBlks)
{
    VSAMapContent* vsaMap = vsaMaps[volumeId];
    return vsaMap->GetDirtyPages(startRba, numBlks);
}

int64_t
VSAMapAPI::GetNumUsedBlocks(int volId)
{
    VSAMapContent* vsaMap = vsaMaps[volId];
    if (vsaMap == nullptr)
    {
        return -(int64_t)POS_EVENT_ID::VSAMAP_NULL_PTR;
    }

    return vsaMap->GetNumUsedBlocks();
}

int
VSAMapAPI::_UpdateVsaMap(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    int ret = 0;
    VSAMapContent* vsaMap = vsaMaps[volumeId];

    for (uint32_t blkIdx = 0; blkIdx < virtualBlks.numBlks; blkIdx++)
    {
        VirtualBlkAddr targetVsa = {.stripeId = virtualBlks.startVsa.stripeId,
            .offset = virtualBlks.startVsa.offset + blkIdx};
        BlkAddr targetRba = startRba + blkIdx;
        ret = vsaMap->SetEntry(targetRba, targetVsa);
        if (ret < 0)
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::VSAMAP_SET_FAILURE, "VSAMap set failure, volumeId:{}  targetRba:{}  targetVsa.sid:{}  targetVsa.offset:{}",
                            volumeId, targetRba, targetVsa.stripeId, targetVsa.offset);
            break;
        }
    }
    return ret;
}

VirtualBlkAddr
VSAMapAPI::_ReadVSA(int volumeId, BlkAddr rba)
{
    VSAMapContent* vsaMap = vsaMaps[volumeId];
    return vsaMap->GetEntry(rba);
}


bool
VSAMapAPI::_IsVSAMapLoaded(int volID)
{
    if (vsaMaps[volID] == nullptr)
    {
        return false;
    }

    if (vsaMaps[volID]->IsLoaded())
    {
        return true;
    }
    else
    {
        return (iVSAMapInternal->GetLoadDoneFlag(volID) == LOAD_DONE);
    }
}

void
VSAMapAPI::EnableVsaMapAccess(int volID)
{
    isVsaMapAccessable[volID] = true;
}

void
VSAMapAPI::DisableVsaMapAccess(int volID)
{
    isVsaMapAccessable[volID] = false;
}

void
VSAMapAPI::EnableVsaMapInternalAccess(int volID)
{
    isVsaMapInternalAccessable[volID] = true;
}

void
VSAMapAPI::DisableVsaMapInternalAccess(int volID)
{
    isVsaMapInternalAccessable[volID] = false;
}

} // namespace pos

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

#include "src/io/general_io/translator.h"

#include <list>

#include "src/allocator_service/allocator_service.h"
#include "src/array/service/array_service_layer.h"
#include "src/include/address_type.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/volume/volume_list.h"

namespace pos
{
thread_local StripeId Translator::recentVsid = UNMAP_STRIPE;
thread_local StripeId Translator::recentLsid = UNMAP_STRIPE;

Translator::Translator(uint32_t volumeId, BlkAddr startRba, uint32_t blockCount,
    std::string& arrayName, bool isRead, IVSAMap* iVSAMap_, IStripeMap* iStripeMap_, IWBStripeAllocator* iWBStripeAllocator_)
: iVSAMap(iVSAMap_),
  iStripeMap(iStripeMap_),
  iWBStripeAllocator(iWBStripeAllocator_),
  startRba(startRba),
  blockCount(blockCount),
  lastVsa(UNMAP_VSA),
  lastLsidEntry{IN_USER_AREA, UNMAP_STRIPE},
  isRead(isRead),
  volumeId(volumeId),
  arrayName(arrayName)
{
    if (nullptr == iVSAMap)
    {
        iVSAMap = MapperServiceSingleton::Instance()->GetIVSAMap(arrayName);
    }
    if (nullptr == iStripeMap)
    {
        iStripeMap = MapperServiceSingleton::Instance()->GetIStripeMap(arrayName);
    }
    if (nullptr == iWBStripeAllocator)
    {
        iWBStripeAllocator = AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(arrayName);
    }
    if (unlikely(volumeId >= MAX_VOLUME_COUNT))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::TRSLTR_WRONG_VOLUME_ID;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId));
        throw eventId;
    }
    iVSAMap->GetVSAs(volumeId, startRba, blockCount, vsaArray);
    if (likely(iStripeMap != nullptr))
    {
        for (uint32_t blockIndex = 0; blockIndex < blockCount; blockIndex++)
        {
            BlkAddr rba = startRba + blockIndex;
            VirtualBlkAddr vsa = vsaArray[blockIndex];
            lsidRefResults[blockIndex] = _GetLsidRefResult(rba, vsa);
        }
    }
}

Translator::Translator(const VirtualBlkAddr& vsa, std::string& arrayName)
: iVSAMap(MapperServiceSingleton::Instance()->GetIVSAMap(arrayName)),
  iStripeMap(MapperServiceSingleton::Instance()->GetIStripeMap(arrayName)),
  iWBStripeAllocator(AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(arrayName)),
  startRba(0),
  blockCount(ONLY_ONE),
  lastVsa(UNMAP_VSA),
  lastLsidEntry{IN_USER_AREA, UNMAP_STRIPE},
  isRead(false),
  volumeId(UINT32_MAX),
  arrayName(arrayName)
{
    vsaArray[0] = vsa;
    if (likely(iStripeMap != nullptr))
    {
        lsidRefResults[0] = _GetLsidRefResult(startRba, vsaArray[0]);
    }
}

Translator::Translator(uint32_t volumeId, BlkAddr rba, std::string& arrayName, bool isRead)
: Translator(volumeId, rba, ONLY_ONE, arrayName, isRead)
{
}

VirtualBlkAddr
Translator::GetVsa(uint32_t blockIndex)
{
    if (unlikely(blockIndex >= blockCount))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::TRSLTR_WRONG_ACCESS;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId));
        return UNMAP_VSA;
    }

    return vsaArray[blockIndex];
}

StripeAddr
Translator::GetLsidEntry(uint32_t blockIndex)
{
    return std::get<0>(GetLsidRefResult(blockIndex));
}

LsidRefResult
Translator::GetLsidRefResult(uint32_t blockIndex)
{
    if (unlikely(blockIndex >= blockCount))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::TRSLTR_WRONG_ACCESS;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId));
        StripeAddr unmapLsid;
        unmapLsid.stripeId = UNMAP_STRIPE;
        unmapLsid.stripeLoc = IN_USER_AREA;
        return std::make_tuple(unmapLsid, false);
    }

    return lsidRefResults[blockIndex];
}

LsidRefResult
Translator::_GetLsidRefResult(BlkAddr rba, VirtualBlkAddr& vsa)
{
    StripeAddr lsidEntry;
    bool referenced = false;

    if (IsUnMapVsa(vsa))
    {
        vsa = iVSAMap->GetRandomVSA(rba);
        lsidEntry.stripeId = iStripeMap->GetRandomLsid(vsa.stripeId);
        lsidEntry.stripeLoc = IN_USER_AREA;
    }
    else
    {
        if (lastVsa.stripeId != vsa.stripeId)
        {
            if (isRead)
            {
                std::tie(lsidEntry, referenced) = iStripeMap->GetLSAandReferLsid(vsa.stripeId);
            }
            else
            {
                lsidEntry = iStripeMap->GetLSA(vsa.stripeId);

		// TODO: Need to Consider Multi Array for thread local variable "recentVsid" 
		// Will apply recentVsid again when it is possible to distinguish array by number for better performanc
		//if (IsUnMapStripe(recentVsid) || vsa.stripeId != recentVsid)
		//{
                //    lsidEntry = iStripeMap->GetLSA(vsa.stripeId);
                //    recentVsid = vsa.stripeId;
                //    recentLsid = lsidEntry.stripeId;
		//}
                //else
                //{
                //    lsidEntry.stripeId = Translator::recentLsid;
                //    lsidEntry.stripeLoc = IN_WRITE_BUFFER_AREA;
                //}
            }
        }
        else
        {
            lsidEntry = lastLsidEntry;
            if (isRead)
            {
                iWBStripeAllocator->ReferLsidCnt(lsidEntry);
            }
        }
    }
    lastVsa = vsa;
    lastLsidEntry = lsidEntry;

    return std::make_tuple(lsidEntry, referenced);
}

bool
Translator::IsMapped(void)
{
    return !IsUnmapped();
}

bool
Translator::IsUnmapped(void)
{
    _CheckSingleBlock();
    return IsUnMapVsa(vsaArray[0]);
}

PhysicalBlkAddr
Translator::GetPba(uint32_t blockIndex)
{
    LogicalBlkAddr lsa = _GetLsa(blockIndex);
    PartitionType partitionType = _GetPartitionType(blockIndex);
    PhysicalBlkAddr pba;
    IIOTranslator* translator = ArrayService::Instance()->Getter()->GetTranslator();
    int ret = translator->Translate(arrayName, partitionType, pba, lsa);
    if (unlikely(ret != 0))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::TRANSLATE_CONVERT_FAIL;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId));
        throw eventId;
    }

    return pba;
}

LogicalBlkAddr
Translator::_GetLsa(uint32_t blockIndex)
{
    if (unlikely(blockIndex >= blockCount))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::TRSLTR_INVALID_BLOCK_INDEX;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId));
        throw eventId;
    }

    StripeAddr lsidEntry = GetLsidEntry(blockIndex);
    VirtualBlkAddr& vsa = vsaArray[blockIndex];
    if (IsUnMapVsa(vsa))
    {
        vsa = iVSAMap->GetRandomVSA(startRba + blockIndex);
    }
    LogicalBlkAddr lsa = {.stripeId = lsidEntry.stripeId, .offset = vsa.offset};

    return lsa;
}

PhysicalEntries
Translator::GetPhysicalEntries(void* mem, uint32_t blockCount)
{
    _CheckSingleBlock();
    LogicalBlkAddr lsa = _GetLsa(0);
    PartitionType partitionType = _GetPartitionType(0);

    std::list<BufferEntry> buffers;
    LogicalWriteEntry logicalEntry = {.addr = lsa, .blkCnt = blockCount, .buffers = &buffers};
    BufferEntry buffer(mem, blockCount);

    logicalEntry.buffers->push_back(buffer);

    PhysicalEntries entries;
    IIOTranslator* translator = ArrayService::Instance()->Getter()->GetTranslator();
    int ret = translator->Convert(
        arrayName, partitionType, entries, logicalEntry);
    if (unlikely(ret != 0))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::TRANSLATE_CONVERT_FAIL;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId));
        throw eventId;
    }
    return entries;
}

PhysicalBlkAddr
Translator::GetPba(void)
{
    _CheckSingleBlock();
    return GetPba(0);
}

PartitionType
Translator::_GetPartitionType(uint32_t blockIndex)
{
    if (iStripeMap->IsInUserDataArea(GetLsidEntry(blockIndex)))
    {
        return USER_DATA;
    }
    else
    {
        return WRITE_BUFFER;
    }
}

void
Translator::_CheckSingleBlock(void)
{
    if (unlikely(ONLY_ONE != blockCount))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::TRSLTR_WRONG_ACCESS;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId));
        throw eventId;
    }
}

} // namespace pos

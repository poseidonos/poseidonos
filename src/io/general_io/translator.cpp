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

#include "src/array/array.h"
#include "src/include/address_type.h"
#include "src/include/ibof_event_id.hpp"
#include "src/logger/logger.h"
#include "src/volume/volume_list.h"

namespace ibofos
{
thread_local StripeId Translator::recentVsid = UNMAP_STRIPE;
thread_local StripeId Translator::recentLsid = UNMAP_STRIPE;

Translator::Translator(uint32_t volumeId, BlkAddr startRba, uint32_t blockCount,
    bool isRead)
: mapper(MapperSingleton::Instance()),
  startRba(startRba),
  blockCount(blockCount),
  lastVsa(UNMAP_VSA),
  lastLsidEntry{IN_USER_AREA, UNMAP_STRIPE},
  arrayManager(ArraySingleton::Instance()),
  isRead(isRead),
  volumeId(volumeId)
{
    if (unlikely(volumeId >= MAX_VOLUME_COUNT))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::TRSLTR_WRONG_VOLUME_ID;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
        throw eventId;
    }
    mapper->GetVSAs(volumeId, startRba, blockCount, vsaArray);

    for (uint32_t blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        BlkAddr rba = startRba + blockIndex;
        VirtualBlkAddr vsa = vsaArray[blockIndex];

        lsidRefResults[blockIndex] = _GetLsidRefResult(rba, vsa);
    }
}

Translator::Translator(const VirtualBlkAddr& vsa)
: mapper(MapperSingleton::Instance()),
  startRba(0),
  blockCount(ONLY_ONE),
  lastVsa(UNMAP_VSA),
  lastLsidEntry{IN_USER_AREA, UNMAP_STRIPE},
  arrayManager(ArraySingleton::Instance()),
  isRead(false),
  volumeId(UINT32_MAX)
{
    vsaArray[0] = vsa;
    lsidRefResults[0] = _GetLsidRefResult(startRba, vsaArray[0]);
}

Translator::Translator(uint32_t volumeId, BlkAddr rba, bool isRead)
: Translator(volumeId, rba, ONLY_ONE, isRead)
{
}

VirtualBlkAddr
Translator::GetVsa(void)
{
    _CheckSingleBlock();
    return GetVsa(0);
}

VirtualBlkAddr
Translator::GetVsa(uint32_t blockIndex)
{
    if (unlikely(blockIndex >= blockCount))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::TRSLTR_WRONG_ACCESS;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
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
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::TRSLTR_WRONG_ACCESS;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
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
        vsa = mapper->GetRandomVSA(rba);
        lsidEntry.stripeId = mapper->GetRandomLsid(vsa.stripeId);
        lsidEntry.stripeLoc = IN_USER_AREA;
    }
    else
    {
        if (lastVsa.stripeId != vsa.stripeId)
        {
            if (isRead)
            {
                std::tie(lsidEntry, referenced) = mapper->GetAndReferLsid(vsa.stripeId);
            }
            else
            {
                if (IsUnMapStripe(recentVsid) || vsa.stripeId != recentVsid)
                {
                    lsidEntry = mapper->GetLSA(vsa.stripeId);
                    recentVsid = vsa.stripeId;
                    recentLsid = lsidEntry.stripeId;
                }
                else
                {
                    lsidEntry.stripeId = Translator::recentLsid;
                    lsidEntry.stripeLoc = IN_WRITE_BUFFER_AREA;
                }
            }
        }
        else
        {
            lsidEntry = lastLsidEntry;
            if (isRead)
            {
                mapper->ReferLsid(lsidEntry);
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
    int ret = arrayManager->Translate(partitionType, pba, lsa);
    if (unlikely(ret != 0))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::TRANSLATE_CONVERT_FAIL;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
        throw eventId;
    }

    return pba;
}

LogicalBlkAddr
Translator::_GetLsa(uint32_t blockIndex)
{
    if (unlikely(blockIndex >= blockCount))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::TRSLTR_INVALID_BLOCK_INDEX;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
        throw eventId;
    }

    StripeAddr lsidEntry = GetLsidEntry(blockIndex);
    VirtualBlkAddr& vsa = vsaArray[blockIndex];
    if (IsUnMapVsa(vsa))
    {
        vsa = mapper->GetRandomVSA(startRba + blockIndex);
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
    int ret = arrayManager->Convert(partitionType, entries, logicalEntry);
    if (unlikely(ret != 0))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::TRANSLATE_CONVERT_FAIL;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
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
    if (mapper->IsInUserDataArea(GetLsidEntry(blockIndex)))
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
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::TRSLTR_WRONG_ACCESS;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
        throw eventId;
    }
}

} // namespace ibofos

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

#include "src/io/general_io/volume_io.h"

#include "src/device/event_framework_api.h"
#include "src/include/ibof_event_id.hpp"
#include "src/logger/logger.h"
#include "src/volume/volume_list.h"

namespace ibofos
{
const StripeAddr VolumeIo::INVALID_LSID_ENTRY =
    {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = UNMAP_STRIPE};
const VirtualBlkAddr VolumeIo::INVALID_VSA = {.stripeId = UNMAP_STRIPE,
    .offset = 0};

VolumeIo::VolumeIo(void* buffer, uint32_t unitCount)
: Ubio(buffer, unitCount),
  volumeId(MAX_VOLUME_COUNT),
  originCore(EventFrameworkApi::GetCurrentReactor()),
  lsidEntry(INVALID_LSID_ENTRY),
  oldLsidEntry(INVALID_LSID_ENTRY),
  vsa(INVALID_VSA),
  allocatedBlockCount(0),
  alignmentInformation({.offset = UINT32_MAX, .size = UINT32_MAX}),
  isGc(false),
  oldVsaForGc(INVALID_VSA)
{
}

VolumeIo::VolumeIo(const VolumeIo& volumeIo)
: Ubio(volumeIo),
  volumeId(volumeIo.volumeId),
  originCore(volumeIo.originCore),
  lsidEntry(INVALID_LSID_ENTRY),
  oldLsidEntry(INVALID_LSID_ENTRY),
  vsa(INVALID_VSA),
  allocatedBlockCount(0),
  alignmentInformation({.offset = UINT32_MAX, .size = UINT32_MAX}),
  isGc(volumeIo.isGc),
  oldVsaForGc(volumeIo.oldVsaForGc)
{
}

VolumeIo::~VolumeIo(void)
{
}

VolumeIoSmartPtr
VolumeIo::Split(uint32_t sectors, bool removalFromTail)
{
    VolumeIoSmartPtr newVolumeIo(new VolumeIo(*this));
    _ReflectSplit(newVolumeIo, sectors, removalFromTail);

    return newVolumeIo;
}

VolumeIoSmartPtr
VolumeIo::GetOriginVolumeIo(void)
{
    VolumeIoSmartPtr originVolumeIo =
        std::dynamic_pointer_cast<VolumeIo>(GetOriginUbio());
    return originVolumeIo;
}

bool
VolumeIo::IsPollingNecessary(void)
{
    if (dir == UbioDir::Read)
    {
        return true;
    }
#if defined NVMe_FLUSH_HANDLING
    else if (dir == UbioDir::Flush)
    {
        return false;
    }
#endif
    else
    {
        if (GetSectorOffsetInBlock(GetRba()) != 0)
        {
            return true;
        }
        uint64_t endAddress = GetRba() + ChangeByteToSector(GetSize());

        if (GetSectorOffsetInBlock(endAddress) != 0)
        {
            return true;
        }
    }

    return false;
}

uint32_t
VolumeIo::GetVolumeId(void)
{
    if (unlikely(false == _CheckVolumeIdSet()))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_VOLUME_ID,
            IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_VOLUME_ID));
        throw IBOF_EVENT_ID::UBIO_INVALID_VOLUME_ID;
    }

    return volumeId;
}

void
VolumeIo::SetVolumeId(uint32_t inputVolumeId)
{
    if (unlikely(_IsInvalidVolumeId(inputVolumeId)))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_VOLUME_ID,
            IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_VOLUME_ID));
        return;
    }

    volumeId = inputVolumeId;
}

bool
VolumeIo::_CheckVolumeIdSet(void)
{
    bool isVolumeIdSet = (false == _IsInvalidVolumeId(volumeId));
    return isVolumeIdSet;
}

bool
VolumeIo::_IsInvalidVolumeId(uint32_t inputVolumeId)
{
    bool isInvalidVolumeId = (inputVolumeId >= MAX_VOLUME_COUNT);
    return isInvalidVolumeId;
}

bool
VolumeIo::_CheckOriginCoreSet(void)
{
    bool isOriginCoreSet = (INVALID_CORE != originCore);
    return isOriginCoreSet;
}

uint32_t
VolumeIo::GetOriginCore(void)
{
    if (false == _CheckOriginCoreSet())
    {
        return INVALID_CORE;
    }

    return originCore;
}

void
VolumeIo::SetLsidEntry(StripeAddr& inputLsidEntry)
{
    if (unlikely(_IsInvalidLsidEntry(inputLsidEntry)))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_LSID,
            IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_LSID));
        return;
    }

    lsidEntry = inputLsidEntry;
}

void
VolumeIo::SetOldLsidEntry(StripeAddr& inputLsidEntry)
{
    if (unlikely(_IsInvalidLsidEntry(inputLsidEntry)))
    {
        assert(false);
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_LSID,
            IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_LSID));
        return;
    }
    oldLsidEntry = inputLsidEntry;
}

const StripeAddr&
VolumeIo::GetLsidEntry(void)
{
    if (unlikely(_IsInvalidLsidEntry(lsidEntry)))
    {
        assert(false);
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_LSID,
            IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_LSID));
        throw IBOF_EVENT_ID::UBIO_INVALID_PBA;
    }
    return lsidEntry;
}

const StripeAddr&
VolumeIo::GetOldLsidEntry(void)
{
    if (unlikely(_IsInvalidLsidEntry(oldLsidEntry)))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_PBA,
            IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_LSID));
        throw IBOF_EVENT_ID::UBIO_INVALID_PBA;
    }
    return oldLsidEntry;
}

bool
VolumeIo::_IsInvalidLsidEntry(StripeAddr& inputLsidEntry)
{
    bool isValid = (inputLsidEntry.stripeId == UNMAP_STRIPE);
    return isValid;
}

const VirtualBlkAddr&
VolumeIo::GetVsa(void)
{
    if (unlikely(false == _CheckVsaSet()))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UBIO_INVALID_VSA;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
    }

    return vsa;
}

void
VolumeIo::SetVsa(VirtualBlkAddr& inputVsa)
{
    if (unlikely(_IsInvalidVsa(inputVsa)))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UBIO_INVALID_VSA;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
        return;
    }
    vsa = inputVsa;
}

bool
VolumeIo::_CheckVsaSet(void)
{
    bool isVsaSet = (false == _IsInvalidVsa(vsa));
    return isVsaSet;
}

bool
VolumeIo::_IsInvalidVsa(VirtualBlkAddr& inputVsa)
{
    if (inputVsa.stripeId == UNMAP_STRIPE && inputVsa.offset == 0)
    {
        return true;
    }
    return false;
}

void
VolumeIo::AddAllocatedVirtualBlks(VirtualBlks& virtualBlocks)
{
    allocatedBlockCount += virtualBlocks.numBlks;
    allocatedVirtualBlks.push_back(virtualBlocks);
}

VirtualBlkAddr
VolumeIo::PopHeadVsa(void)
{
    VirtualBlks& firstVsaRange = allocatedVirtualBlks.front();
    VirtualBlkAddr headVsa = firstVsaRange.startVsa;
    firstVsaRange.numBlks--;
    firstVsaRange.startVsa.offset++;

    return headVsa;
}

VirtualBlkAddr
VolumeIo::PopTailVsa(void)
{
    VirtualBlks& lastVsaRange = allocatedVirtualBlks.back();
    VirtualBlkAddr tailVsa = lastVsaRange.startVsa;
    tailVsa.offset += lastVsaRange.numBlks - 1;
    lastVsaRange.numBlks--;

    return tailVsa;
}

uint32_t
VolumeIo::GetAllocatedBlockCount(void)
{
    return allocatedBlockCount;
}

uint32_t
VolumeIo::GetAllocatedVirtualBlksCount(void)
{
    return allocatedVirtualBlks.size();
}

VirtualBlks&
VolumeIo::GetAllocatedVirtualBlks(uint32_t index)
{
    return allocatedVirtualBlks[index];
}

void
VolumeIo::SetAlignmentInformation(AlignmentInformation input)
{
    alignmentInformation = input;
}

const AlignmentInformation&
VolumeIo::GetAlignmentInformation(void)
{
    return alignmentInformation;
}

void
VolumeIo::SetGc(VirtualBlkAddr& oldVsa)
{
    isGc = true;
    oldVsaForGc = oldVsa;
}

bool
VolumeIo::IsGc(void)
{
    return isGc;
}

const VirtualBlkAddr&
VolumeIo::GetOldVsa(void)
{
    if (unlikely(_IsInvalidVsa(oldVsaForGc)))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UBIO_INVALID_VSA;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
        return INVALID_VSA;
    }

    return oldVsaForGc;
}

} // namespace ibofos

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

#include "src/bio/volume_io.h"

#include "src/array_mgmt/array_manager.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/include/pos_event_id.hpp"
#include "src/include/core_const.h"
#include "src/include/branch_prediction.h"
#include "src/logger/logger.h"
#include "src/volume/volume_list.h"

namespace pos
{
const StripeAddr VolumeIo::INVALID_LSID_ENTRY =
    {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = UNMAP_STRIPE};
const VirtualBlkAddr VolumeIo::INVALID_VSA = {.stripeId = UNMAP_STRIPE,
    .offset = 0};
const uint64_t VolumeIo::INVALID_RBA = UINT64_MAX;

VolumeIo::VolumeIo(void* buffer, uint32_t unitCount, int arrayId)
: VolumeIo(buffer, unitCount, arrayId, ArrayMgr())
{
}

VolumeIo::VolumeIo(void* buffer, uint32_t unitCount, int arrayId, IArrayMgmt* arrayMgmt)
: Ubio(buffer, unitCount, arrayId),
  volumeId(MAX_VOLUME_COUNT),
  originCore(EventFrameworkApiSingleton::Instance()->GetCurrentReactor()),
  lsidEntry(INVALID_LSID_ENTRY),
  oldLsidEntry(INVALID_LSID_ENTRY),
  vsa(INVALID_VSA),
  sectorRba(INVALID_RBA),
  arrayMgmt(arrayMgmt)
{
}

VolumeIo::VolumeIo(const VolumeIo& volumeIo)
: VolumeIo(volumeIo, ArrayMgr())
{
}

VolumeIo::VolumeIo(const VolumeIo& volumeIo, IArrayMgmt* arrayMgmt)
: Ubio(volumeIo),
  volumeId(volumeIo.volumeId),
  originCore(volumeIo.originCore),
  lsidEntry(INVALID_LSID_ENTRY),
  oldLsidEntry(INVALID_LSID_ENTRY),
  vsa(INVALID_VSA),
  sectorRba(volumeIo.sectorRba),
  arrayMgmt(arrayMgmt)
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

    if (removalFromTail)
    {
        newVolumeIo->sectorRba += ChangeByteToSector(GetSize());
    }
    else
    {
        sectorRba += sectors;
    }


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
    IArrayInfo* arrayInfo = arrayMgmt->GetInfo(this->GetArrayId())->arrayInfo;
    if (arrayInfo->IsWriteThroughEnabled())
    {
        return true;
    }
    else
    {
        if (dir == UbioDir::Read)
        {
            return true;
        }
        else
        {
            if (GetSectorOffsetInBlock(GetSectorRba()) != 0)
            {
                return true;
            }
            uint64_t endAddress = GetSectorRba() + ChangeByteToSector(GetSize());

            if (GetSectorOffsetInBlock(endAddress) != 0)
            {
                return true;
            }
        }
        return false;
    }
}

uint32_t
VolumeIo::GetVolumeId(void)
{
    if (unlikely(false == _CheckVolumeIdSet()))
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::UBIO_INVALID_VOLUME_ID,
            "Invalid volume ID for Ubio");
        throw POS_EVENT_ID::UBIO_INVALID_VOLUME_ID;
    }

    return volumeId;
}

void
VolumeIo::SetVolumeId(uint32_t inputVolumeId)
{
    if (unlikely(_IsInvalidVolumeId(inputVolumeId)))
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::UBIO_INVALID_VOLUME_ID,
            "Invalid volume ID for Ubio");
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
VolumeIo::SetSectorRba(uint64_t inputSectorRba)
{
    if (unlikely(_IsInvalidSectorRba(inputSectorRba)))
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::UBIO_INVALID_RBA,
            "Invalid RBA for Ubio");
        return;
    }

    sectorRba = inputSectorRba;
}

uint64_t
VolumeIo::GetSectorRba(void)
{
    if (unlikely(false == _CheckSectorRbaSet()))
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::UBIO_INVALID_RBA,
            "Invalid RBA for Ubio");
        throw POS_EVENT_ID::UBIO_INVALID_RBA;
    }

    return sectorRba;
}

bool
VolumeIo::_CheckSectorRbaSet(void)
{
    bool isRbaSet = (false == _IsInvalidSectorRba(sectorRba));
    return isRbaSet;
}

bool
VolumeIo::_IsInvalidSectorRba(uint64_t inputSectorRba)
{
    bool isInvalidSectorRba = (inputSectorRba == INVALID_RBA);
    return isInvalidSectorRba;
}

void
VolumeIo::SetLsidEntry(StripeAddr& inputLsidEntry)
{
    if (unlikely(_IsInvalidLsidEntry(inputLsidEntry)))
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::UBIO_INVALID_LSID,
            "Invalid LSID for Ubio");
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
        POS_TRACE_ERROR((int)POS_EVENT_ID::UBIO_INVALID_LSID,
            "Invalid LSID for Ubio");
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
        POS_TRACE_ERROR((int)POS_EVENT_ID::UBIO_INVALID_LSID,
            "Invalid LSID for Ubio");
        throw POS_EVENT_ID::UBIO_INVALID_PBA;
    }
    return lsidEntry;
}

const StripeAddr&
VolumeIo::GetOldLsidEntry(void)
{
    if (unlikely(_IsInvalidLsidEntry(oldLsidEntry)))
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::UBIO_INVALID_PBA,
            "Invalid PBA for Ubio");
        throw POS_EVENT_ID::UBIO_INVALID_PBA;
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
        POS_EVENT_ID eventId = POS_EVENT_ID::UBIO_INVALID_VSA;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Invalid VSA for Ubio");
    }

    return vsa;
}

void
VolumeIo::SetVsa(VirtualBlkAddr& inputVsa)
{
    if (unlikely(_IsInvalidVsa(inputVsa)))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::UBIO_INVALID_VSA;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Invalid VSA for Ubio");
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
VolumeIo::SetUserLsid(StripeId inputStripeId)
{
    if (unlikely(inputStripeId == UNMAP_STRIPE))
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::UBIO_INVALID_LSID,
            "Invalid LSID for Ubio");
        return;
    }
    stripeId = inputStripeId;
}

StripeId
VolumeIo::GetUserLsid(void)
{
    if (unlikely(stripeId == UNMAP_STRIPE))
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::UBIO_INVALID_LSID,
            "Invalid LSID for Ubio");
    }
    return stripeId;
}

} // namespace pos

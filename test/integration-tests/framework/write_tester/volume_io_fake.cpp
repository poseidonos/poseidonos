/*
 *   BSD LICENSE
 *   Copyright (c) 2023 Samsung Electronics Corporation
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

#include "test/integration-tests/framework/write_tester/volume_io_fake.h"
#include "src/include/core_const.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"

namespace pos
{
FakeVolumeIo::~FakeVolumeIo(void)
{
}

FakeVolumeIo::FakeVolumeIo(void *buffer, uint32_t unitCount, int arrayId)
: VolumeIo(buffer, unitCount, arrayId)
{
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    SetCallback(callback);
}

void
FakeVolumeIo::Init(void)
{

}

VolumeIoSmartPtr
FakeVolumeIo::Split(uint32_t sectors, bool tail)
{
    VolumeIoSmartPtr newVolumeIo(new FakeVolumeIo(*this));
    _ReflectSplit(newVolumeIo, sectors, tail);

    if (tail)
    {
        newVolumeIo->AddToSectorRba(ChangeByteToSector(GetSize()));
    }
    else
    {
        sectorRba += sectors;
    }

    return newVolumeIo;
}

VolumeIoSmartPtr
FakeVolumeIo::GetOriginVolumeIo(void)
{
    VolumeIoSmartPtr originVolumeIo =
        std::dynamic_pointer_cast<VolumeIo>(GetOriginUbio());
    return originVolumeIo;
}

bool
FakeVolumeIo::IsPollingNecessary(void)
{
    return false;
}

uint32_t
FakeVolumeIo::GetVolumeId(void)
{
    return volumeId;
}

void
FakeVolumeIo::SetSectorRba(uint64_t inputSectorRba)
{
    sectorRba = inputSectorRba;
}

uint64_t
FakeVolumeIo::GetSectorRba(void)
{
    return sectorRba;
}

void
FakeVolumeIo::SetVolumeId(uint32_t inputVolumeId)
{
    volumeId = inputVolumeId;
}

uint32_t
FakeVolumeIo::GetOriginCore(void)
{
    return originCore;
}

void
FakeVolumeIo::SetLsidEntry(StripeAddr& inputLsidEntry)
{
    lsidEntry = inputLsidEntry;
}

void
FakeVolumeIo::SetOldLsidEntry(StripeAddr& inputLsidEntry)
{
    oldLsidEntry = inputLsidEntry;
}

const StripeAddr&
FakeVolumeIo::GetLsidEntry(void)
{
    return lsidEntry;
}

const StripeAddr&
FakeVolumeIo::GetOldLsidEntry(void)
{
    return oldLsidEntry;
}

const VirtualBlkAddr&
FakeVolumeIo::GetVsa(void)
{
    return vsa;
}

void
FakeVolumeIo::SetVsa(VirtualBlkAddr& inputVsa)
{
    vsa = inputVsa;
}

bool
FakeVolumeIo::_IsInvalidLsidEntry(StripeAddr& inputLsidEntry)
{
    bool isValid = (inputLsidEntry.stripeId == UNMAP_STRIPE);
    return isValid;
}

StripeId
FakeVolumeIo::GetUserLsid(void)
{
    return stripeId;
}

void
FakeVolumeIo::_ReflectSplit(UbioSmartPtr newUbio, uint32_t sectors, bool removalFromTail)
{
    return;
}

} // namespace pos

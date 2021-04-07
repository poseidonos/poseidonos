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

#pragma once

#include "src/io/general_io/merger.h"

namespace ibofos
{
template<typename T>
Merger<T>::Merger(VolumeIoSmartPtr originalVolumeIo)
: mergingSize(0),
  mergingPba{
      0,
  },
  mergingVsa(UNMAP_VSA),
  mergingLsidEntry{IN_USER_AREA, UNMAP_STRIPE},
  splitVolumeIoCount(0),
  originalVolumeIo(originalVolumeIo)
{
}

template<typename T>
void
Merger<T>::Add(PhysicalBlkAddr& pba, VirtualBlkAddr& vsa, StripeAddr& lsidEntry,
    uint32_t targetSize)
{
    if (mergingSize == 0)
    {
        _Start(pba, vsa, lsidEntry, targetSize);
    }
    else if (_IsMergable(pba, vsa))
    {
        mergingSize += targetSize;
    }
    else
    {
        Cut();
        _Start(pba, vsa, lsidEntry, targetSize);
    }
}

template<typename T>
void
Merger<T>::_Start(PhysicalBlkAddr& pba, VirtualBlkAddr& vsa, StripeAddr& lsidEntry,
    uint32_t targetSize)
{
    mergingPba = pba;
    mergingVsa = vsa;
    mergingSize = targetSize;
    mergingLsidEntry = lsidEntry;
}

template<typename T>
bool
Merger<T>::_IsMergable(PhysicalBlkAddr& pba, VirtualBlkAddr& vsa)
{
    bool isSameDevice = (mergingPba.dev == pba.dev);
    uint32_t nextLba = mergingPba.lba + ChangeByteToSector(mergingSize);
    bool isContiguous = (nextLba == pba.lba);
    bool isSameStripe = (mergingVsa.stripeId == vsa.stripeId);

    return isSameDevice & isContiguous & isSameStripe;
}

template<typename T>
VolumeIoSmartPtr
Merger<T>::GetSplit(uint32_t index)
{
    return splitVolumeIo[index];
}

template<typename T>
uint32_t
Merger<T>::GetSplitCount(void)
{
    return splitVolumeIoCount;
}

template<typename T>
void
Merger<T>::Cut(void)
{
    if (mergingSize != 0)
    {
        uint32_t sectorSize = ChangeByteToSector(mergingSize);
        VolumeIoSmartPtr split = originalVolumeIo->Split(sectorSize, false);

        CallbackSmartPtr callee(originalVolumeIo->GetCallback());
        CallbackSmartPtr callback(new T(split));
        callback->SetCallee(callee);
        split->SetCallback(callback);
        split->SetPba(mergingPba);
        split->SetVsa(mergingVsa);
        split->SetLsidEntry(mergingLsidEntry);

        splitVolumeIo[splitVolumeIoCount] = split;
        splitVolumeIoCount++;

        mergingSize = 0;
    }
}

} // namespace ibofos

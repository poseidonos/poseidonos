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

#include <thread>

#include "src/mapper/mapper.h"
#include "src/event_scheduler/event.h"

namespace pos
{
Mapper::Mapper(IArrayInfo* info, IStateControl* iState)
: addrInfo(nullptr),
  vsaMapManager(nullptr),
  stripeMapManager(nullptr),
  reverseMapManager(nullptr),
  iArrayinfo(info),
  iStateControl(iState),
  isInitialized(false)
{
}

Mapper::~Mapper(void)
{
}

int
Mapper::Init(void)
{
    return 0;
}

int
Mapper::FlushAllMaps(void)
{
    return 0;
}

void
Mapper::WaitForFlushAllMapsDone(void)
{
}

void
Mapper::Dispose(void)
{
}

int
Mapper::StoreAllMaps(void)
{
    return 0;
}

void
Mapper::Close(void)
{
}

int
Mapper::FlushDirtyMpages(int mapId, EventSmartPtr callback, MpageList dirtyPages)
{
    return 0;
}

MapContent*
Mapper::_GetMapContent(int mapId)
{
    return nullptr;
}

int
Mapper::ReadVsaMapEntry(int volId, BlkAddr rba, std::string fname)
{
    return 0;
}

int
Mapper::WriteVsaMapEntry(int volId, BlkAddr rba, VirtualBlkAddr vsa)
{
    return 0;
}

int
Mapper::ReadVsaMap(int volId, std::string fname)
{
    return 0;
}

int
Mapper::WriteVsaMap(int volId, std::string fname)
{
    return 0;
}

int
Mapper::ReadStripeMapEntry(StripeId vsid, std::string fname)
{
    return 0;
}

int
Mapper::WriteStripeMapEntry(StripeId vsid, StripeLoc loc, StripeId lsid)
{
    return 0;
}

int
Mapper::ReadReverseMap(StripeId vsid, std::string fname)
{
    return 0;
}

int
Mapper::WriteReverseMap(StripeId vsid, std::string fname)
{
    return 0;
}

int
Mapper::ReadWholeReverseMap(std::string fname)
{
    return 0;
}

int
Mapper::WriteWholeReverseMap(std::string fname)
{
    return 0;
}

int
Mapper::ReadReverseMapEntry(StripeId vsid, BlkOffset offset, std::string fname)
{
    return 0;
}

int
Mapper::WriteReverseMapEntry(StripeId vsid, BlkOffset offset, BlkAddr rba,
    uint32_t volumeId)
{
    return 0;
}

int
Mapper::_LoadReverseMapVsidFromMFS(ReverseMapPack* reverseMapPack, StripeId vsid)
{
    return 0;
}

int
Mapper::_StoreReverseMapToMFS(ReverseMapPack* reverseMapPack)
{
    return 0;
}

int
Mapper::ReadStripeMap(std::string fname)
{
    return 0;
}

int
Mapper::WriteStripeMap(std::string fname)
{
    return 0;
}

int
Mapper::GetMapLayout(std::string fname)
{
    return 0;
}

VSAMapContent*
Mapper::_GetFirstValidVolume(void)
{
    return nullptr;
}

IVSAMap*
Mapper::GetIVSAMap(void)
{
    return nullptr;
}

IStripeMap*
Mapper::GetIStripeMap(void)
{
    return nullptr;
}

IMapFlush*
Mapper::GetIMapFlush(void)
{
    return nullptr;
}

} // namespace pos

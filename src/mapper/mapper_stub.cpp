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

#include "mapper.h"

namespace ibofos
{
Mapper::Mapper(void)
{
}

Mapper::~Mapper(void)
{
}

void
Mapper::Init(void)
{
}

void
Mapper::_InitMetadata(const MapperAddressInfo& info)
{
}

int
Mapper::SyncStore(void)
{
    return 0;
}

void
Mapper::Close(void)
{
}

void
Mapper::RegisterToPublisher(void)
{
}

void
Mapper::RemoveFromPublisher(void)
{
}

VirtualBlkAddr
Mapper::GetVSA(int volumeId, BlkAddr rba)
{
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    return vsa;
}

VirtualBlkAddr
Mapper::GetVSAInternal(int volumeId, BlkAddr rba, int& caller)
{
    caller = OK_READY;
    return UNMAP_VSA;
}

VirtualBlkAddr
Mapper::_ReadVSA(int volumeId, BlkAddr rba)
{
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    return vsa;
}

int
Mapper::GetVSAs(int volumeId, BlkAddr startRba, uint32_t numBlks,
    VsaArray& vsaArray)
{
    return 0;
}

int
Mapper::SetVsaMap(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    return 0;
}

int
Mapper::SetVsaMapInternal(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    return 0;
}

int
Mapper::_UpdateVsaMap(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    return 0;
}

MpageList
Mapper::GetDirtyVsaMapPages(int volumeId, BlkAddr startRba, uint64_t numBlks)
{
    MpageList list;
    return list;
}

int
Mapper::LinkReverseMap(Stripe* stripe, StripeId wbLsid, StripeId vsid)
{
    return 0;
}

int
Mapper::ResetVSARange(int volumeId, BlkAddr rba, uint64_t cnt)
{
    return 0;
}

VirtualBlkAddr
Mapper::GetRandomVSA(BlkAddr rba)
{
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    return vsa;
}

StripeId
Mapper::GetRandomLsid(StripeId vsid)
{
    return 0;
}

StripeAddr
Mapper::GetLSA(StripeId vsid)
{
    StripeAddr entry = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = 0};
    return entry;
}

int
Mapper::UpdateStripeMap(StripeId vsid, StripeId lsid, StripeLoc loc)
{
    return 0;
}

MpageList
Mapper::GetDirtyStripeMapPages(int vsid)
{
    MpageList list;
    return list;
}

int
Mapper::StartDirtyPageFlush(int mapId, MpageList dirtyPages, EventSmartPtr callbackEvent)
{
    int result = callbackEvent->Execute();
    return (result == 0);
}

int
Mapper::FlushMap(int mapId, EventSmartPtr callback)
{
    return 0;
}

MapContent*
Mapper::_GetMapContent(int mapId)
{
    return nullptr;
}

std::tuple<StripeAddr, bool>
Mapper::GetAndReferLsid(StripeId vsid)
{
    StripeAddr stripeAddr;
    bool referenced = false;
    return std::make_tuple(stripeAddr, referenced);
}

bool
Mapper::ReferLsid(StripeAddr& lsidEntry)
{
    return true;
}

void
Mapper::DereferLsid(StripeAddr& lsidEntry, uint32_t blockCount)
{
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

int64_t
Mapper::GetNumUsedBlocks(int volId)
{
    return 0;
}

MetaFileIntf*
Mapper::GetRevMapWholeFile(void)
{
    return nullptr;
}

} // namespace ibofos

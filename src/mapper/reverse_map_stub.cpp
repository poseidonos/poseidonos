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

#include "reverse_map.h"

namespace ibofos
{
uint64_t ReverseMapPack::mpageSize;
uint64_t ReverseMapPack::numMpagesPerStripe;
uint64_t ReverseMapPack::fileSizePerStripe;

ReverseMapPack::ReverseMapPack(void)
{
}

ReverseMapPack::~ReverseMapPack(void)
{
}

int
ReverseMapPack::SetPageSize(StorageOpt storageOpt)
{
    return 0;
}

int
ReverseMapPack::SetNumMpages(void)
{
    return 0;
}

void
ReverseMapPack::Init(StripeId wblsid)
{
}

uint64_t
ReverseMapPack::GetfileSizePerStripe(void)
{
    return 0;
}

int
ReverseMapPack::LinkVsid(StripeId vsid)
{
    return 0;
}

int
ReverseMapPack::UnLinkVsid(void)
{
    return 0;
}

int
ReverseMapPack::Load(EventSmartPtr callback)
{
    return 0;
}

int
ReverseMapPack::Flush(Stripe* stripe, EventSmartPtr callback)
{
    return 0;
}

void
ReverseMapPack::_RevMapPageIoDone(AsyncMetaFileIoCtx* ctx)
{
}

std::tuple<uint32_t, uint32_t, uint32_t>
ReverseMapPack::_ReverseMapGeometry(uint64_t offset) //   Ex) offset = 300 | 500
{
    return std::make_tuple(0, 0, 0);
}

int
ReverseMapPack::SetReverseMapEntry(uint32_t offset, BlkAddr rba,
    uint32_t volumeId)
{
    return 0;
}

std::tuple<BlkAddr, uint32_t>
ReverseMapPack::GetReverseMapEntry(uint32_t offset)
{
    return std::make_tuple(0, 0);
}

int
ReverseMapPack::IsAsyncIoDone(void)
{
    return 0;
}

void
ReverseMapPack::_HeaderInit(StripeId wblsid)
{
}

std::tuple<uint32_t, uint32_t>
ReverseMapPack::_GetCurrentTime(void)
{
    return std::make_tuple(0, 0);
}

int
ReverseMapPack::_SetTimeToHeader(ACTION act)
{
    return 0;
}

int
ReverseMapPack::_SetNumPages(int setValue)
{
    return 0;
}

void
ReverseMapPack::_SetRevMapFile(MetaFileIntf* setFile)
{
}

int
ReverseMapPack::WbtFileSyncIo(MetaFileIntf* fileLinux, MetaFsIoOpcode IoDirection)
{
    return 0;
}

} // namespace ibofos

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

#include "src/allocator/allocator.h"
#include "src/event_scheduler/event.h"
#include "src/include/smart_ptr_type.h"
#include "src/allocator/block_manager/block_allocator_stub.h"
#include "src/array_models/interface/i_array_info.h"

namespace pos
{
Allocator::Allocator(IArrayInfo* info, IStateControl* iState)
: VolumeEvent("Allocator", info->GetName()),
  addrInfo(nullptr),
  contextManager(nullptr),
  blockManager(nullptr),
  wbStripeManager(nullptr),
  isInitialized(false),
  iArrayInfo(info),
  iStateControl(iState)
{
}

Allocator::~Allocator(void)
{
}

int
Allocator::Init(void)
{
    return 0;
}

void
Allocator::Dispose(void)
{
}

void
Allocator::SetGcThreshold(uint32_t inputThreshold)
{
}

void
Allocator::SetUrgentThreshold(uint32_t inputThreshold)
{
}

bool
Allocator::VolumeUnmounted(std::string volName, int volID, std::string arrayName)
{
    return true;
}

int
Allocator::GetMeta(AllocatorCtxType type, std::string fname)
{
    return 0;
}

int
Allocator::SetMeta(AllocatorCtxType type, std::string fname)
{
    return 0;
}

int
Allocator::GetBitmapLayout(std::string fname)
{
    return 0;
}

int
Allocator::GetInstantMetaInfo(std::string fname)
{
    return 0;
}

void
Allocator::FlushAllUserdataWBT(void)
{
}

IBlockAllocator*
Allocator::GetIBlockAllocator(void)
{
    IBlockAllocator* iBlockAllocator = new StubBlockAllocator;
    return iBlockAllocator;
}

IWBStripeAllocator*
Allocator::GetIWBStripeAllocator(void)
{
    return nullptr;
}

IAllocatorCtx*
Allocator::GetIAllocatorCtx(void)
{
    return nullptr;
}

ISegmentCtx*
Allocator::GetISegmentCtx(void)
{
    return nullptr;
}

IWBStripeCtx*
Allocator::GetIWBStripeCtx(void)
{
    return nullptr;
}

} // namespace pos

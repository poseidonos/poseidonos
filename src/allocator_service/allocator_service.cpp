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

#include "src/allocator_service/allocator_service.h"

namespace pos
{
void
AllocatorService::RegisterAllocator(std::string arrayName, IBlockAllocator* iblockAllocator)
{
    iBlockAllocator.Register(arrayName, iblockAllocator);
}

void
AllocatorService::RegisterAllocator(std::string arrayName, IWBStripeAllocator* iwbstripeAllocator)
{
    iWBStripeAllocator.Register(arrayName, iwbstripeAllocator);
}

void
AllocatorService::RegisterAllocator(std::string arrayName, IWBStripeCtx* iwbstripeCtx)
{
    iWBStripeCtx.Register(arrayName, iwbstripeCtx);
}

void
AllocatorService::RegisterAllocator(std::string arrayName, ISegmentCtx* isegmentCtx)
{
    iSegmentCtx.Register(arrayName, isegmentCtx);
}

void
AllocatorService::RegisterAllocator(std::string arrayName, IRebuildCtx* irebuildCtx)
{
    iRebuildCtx.Register(arrayName, irebuildCtx);
}

void
AllocatorService::RegisterAllocator(std::string arrayName, IAllocatorCtx* iallocatorCtx)
{
    iAllocatorCtx.Register(arrayName, iallocatorCtx);
}

void
AllocatorService::RegisterAllocator(std::string arrayName, IAllocatorWbt* iallocatorWbt)
{
    iAllocatorWbt.Register(arrayName, iallocatorWbt);
}

void
AllocatorService::UnregisterAllocator(std::string arrayName)
{
    iBlockAllocator.Unregister(arrayName);
    iWBStripeAllocator.Unregister(arrayName);
    iWBStripeCtx.Unregister(arrayName);
    iSegmentCtx.Unregister(arrayName);
    iRebuildCtx.Unregister(arrayName);
    iAllocatorCtx.Unregister(arrayName);
    iAllocatorWbt.Unregister(arrayName);
}

IBlockAllocator*
AllocatorService::GetIBlockAllocator(std::string arrayName)
{
    return iBlockAllocator.GetInterface(arrayName);
}

IWBStripeAllocator*
AllocatorService::GetIWBStripeAllocator(std::string arrayName)
{
    return iWBStripeAllocator.GetInterface(arrayName);
}

IWBStripeCtx*
AllocatorService::GetIWBStripeCtx(std::string arrayName)
{
    return iWBStripeCtx.GetInterface(arrayName);
}

ISegmentCtx*
AllocatorService::GetISegmentCtx(std::string arrayName)
{
    return iSegmentCtx.GetInterface(arrayName);
}

IRebuildCtx*
AllocatorService::GetIRebuildCtx(std::string arrayName)
{
    return iRebuildCtx.GetInterface(arrayName);
}

IAllocatorCtx*
AllocatorService::GetIAllocatorCtx(std::string arrayName)
{
    return iAllocatorCtx.GetInterface(arrayName);
}

IAllocatorWbt*
AllocatorService::GetIAllocatorWbt(std::string arrayName)
{
    return iAllocatorWbt.GetInterface(arrayName);
}

} // namespace pos

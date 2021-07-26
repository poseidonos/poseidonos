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
AllocatorService::RegisterAllocator(std::string arrayName, IAllocatorWbt* iallocatorWbt)
{
    iAllocatorWbt.Register(arrayName, iallocatorWbt);
}

void
AllocatorService::RegisterAllocator(std::string arrayName, IContextManager* icontextManager)
{
    iContextManager.Register(arrayName, icontextManager);
}

void
AllocatorService::RegisterAllocator(std::string arrayName, IContextReplayer* icontextReplayer)
{
    iContextReplayer.Register(arrayName, icontextReplayer);
}

void
AllocatorService::UpdateAllocator(std::string arrayName, void* allocator)
{
    allocators.Update(arrayName, allocator);
}

void
AllocatorService::UnregisterAllocator(std::string arrayName)
{
    iBlockAllocator.Unregister(arrayName);
    iWBStripeAllocator.Unregister(arrayName);
    iAllocatorWbt.Unregister(arrayName);
    iContextManager.Unregister(arrayName);
    iContextReplayer.Unregister(arrayName);
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

IAllocatorWbt*
AllocatorService::GetIAllocatorWbt(std::string arrayName)
{
    return iAllocatorWbt.GetInterface(arrayName);
}

IContextManager*
AllocatorService::GetIContextManager(std::string arrayName)
{
    return iContextManager.GetInterface(arrayName);
}

IContextReplayer*
AllocatorService::GetIContextReplayer(std::string arrayName)
{
    return iContextReplayer.GetInterface(arrayName);
}

} // namespace pos

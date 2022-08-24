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

#include "src/allocator_service/allocator_service.h"
#include "src/logger/logger.h"

namespace pos
{
AllocatorService::AllocatorService(void)
{
    iBlockAllocator.fill(nullptr);
    iWBStripeAllocator.fill(nullptr);
    iAllocatorWbt.fill(nullptr);
    iContextManager.fill(nullptr);
    iContextReplayer.fill(nullptr);
}

void
AllocatorService::RegisterAllocator(std::string arrayName, int arrayId,
    IBlockAllocator* iblockAllocator, IWBStripeAllocator* iwbstripeAllocator,
    IAllocatorWbt* iallocatorWbt, IContextManager* icontextManager, IContextReplayer* icontextReplayer)
{
    if (arrayNameToId.find(arrayName) == arrayNameToId.end())
    {
        arrayNameToId.emplace(arrayName, arrayId);

        iBlockAllocator[arrayId] = iblockAllocator;
        iWBStripeAllocator[arrayId] = iwbstripeAllocator;
        iAllocatorWbt[arrayId] = iallocatorWbt;
        iContextManager[arrayId] = icontextManager;
        iContextReplayer[arrayId] = icontextReplayer;
    }
    else
    {
        POS_TRACE_ERROR(EID(MAPPER_ALREADY_EXIST), "Allocator for array {} is already registered", arrayName);
    }
}

void
AllocatorService::UnregisterAllocator(std::string arrayName)
{
    if (arrayNameToId.find(arrayName) != arrayNameToId.end())
    {
        int arrayId = arrayNameToId[arrayName];
        arrayNameToId.erase(arrayName);

        iBlockAllocator[arrayId] = nullptr;
        iWBStripeAllocator[arrayId] = nullptr;
        iAllocatorWbt[arrayId] = nullptr;
        iContextManager[arrayId] = nullptr;
        iContextReplayer[arrayId] = nullptr;
    }
    else
    {
        POS_TRACE_INFO(EID(MAPPER_ALREADY_EXIST), "Allocator for array {} already unregistered", arrayName);
    }
}

IBlockAllocator*
AllocatorService::GetIBlockAllocator(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return iBlockAllocator[arrayId->second];
    }
}

IWBStripeAllocator*
AllocatorService::GetIWBStripeAllocator(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return iWBStripeAllocator[arrayId->second];
    }
}

IAllocatorWbt*
AllocatorService::GetIAllocatorWbt(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return iAllocatorWbt[arrayId->second];
    }
}

IContextManager*
AllocatorService::GetIContextManager(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return iContextManager[arrayId->second];
    }
}

IContextReplayer*
AllocatorService::GetIContextReplayer(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return iContextReplayer[arrayId->second];
    }
}

IBlockAllocator*
AllocatorService::GetIBlockAllocator(int arrayId)
{
    return iBlockAllocator[arrayId];
}

IWBStripeAllocator*
AllocatorService::GetIWBStripeAllocator(int arrayId)
{
    return iWBStripeAllocator[arrayId];
}

IAllocatorWbt*
AllocatorService::GetIAllocatorWbt(int arrayId)
{
    return iAllocatorWbt[arrayId];
}

IContextManager*
AllocatorService::GetIContextManager(int arrayId)
{
    return iContextManager[arrayId];
}

IContextReplayer*
AllocatorService::GetIContextReplayer(int arrayId)
{
    return iContextReplayer[arrayId];
}

} // namespace pos

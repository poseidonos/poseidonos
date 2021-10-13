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

#pragma once

#include <array>
#include <string>
#include <unordered_map>

#include "src/include/array_mgmt_policy.h"
#include "src/lib/singleton.h"

namespace pos
{
class IBlockAllocator;
class IWBStripeAllocator;
class IRebuildCtx;
class IAllocatorWbt;
class IContextManager;
class IContextReplayer;

class AllocatorService
{
    friend class Singleton<AllocatorService>;

public:
    AllocatorService(void);
    virtual ~AllocatorService(void) = default;

    void RegisterAllocator(std::string arrayName, int arrayId,
        IBlockAllocator* iBlockAllocator, IWBStripeAllocator* iWBStripeAllocator,
        IAllocatorWbt* iAllocatorWbt, IContextManager* iContextManager,
        IContextReplayer* iContextReplayer);
    void UnregisterAllocator(std::string arrayName);

    virtual IBlockAllocator* GetIBlockAllocator(std::string arrayName);
    virtual IWBStripeAllocator* GetIWBStripeAllocator(std::string arrayName);
    IAllocatorWbt* GetIAllocatorWbt(std::string arrayName);
    IContextManager* GetIContextManager(std::string arrayName);
    IContextReplayer* GetIContextReplayer(std::string arrayName);

    virtual IBlockAllocator* GetIBlockAllocator(int arrayId);
    virtual IWBStripeAllocator* GetIWBStripeAllocator(int arrayId);
    IAllocatorWbt* GetIAllocatorWbt(int arrayId);
    IContextManager* GetIContextManager(int arrayId);
    IContextReplayer* GetIContextReplayer(int arrayId);

private:
    std::unordered_map<std::string, int> arrayNameToId;

    std::array<IBlockAllocator*, ArrayMgmtPolicy::MAX_ARRAY_CNT> iBlockAllocator;
    std::array<IWBStripeAllocator*, ArrayMgmtPolicy::MAX_ARRAY_CNT> iWBStripeAllocator;
    std::array<IAllocatorWbt*, ArrayMgmtPolicy::MAX_ARRAY_CNT> iAllocatorWbt;
    std::array<IContextManager*, ArrayMgmtPolicy::MAX_ARRAY_CNT> iContextManager;
    std::array<IContextReplayer*, ArrayMgmtPolicy::MAX_ARRAY_CNT> iContextReplayer;
};

using AllocatorServiceSingleton = Singleton<AllocatorService>;

} // namespace pos

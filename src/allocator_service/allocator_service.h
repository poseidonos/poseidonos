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

#include "src/lib/singleton.h"
#include "src/allocator_service/allocator_interface_container.h"

#include <string>

namespace pos
{
class IBlockAllocator;
class IWBStripeAllocator;
class IWBStripeCtx;
class ISegmentCtx;
class IRebuildCtx;
class IAllocatorCtx;
class IAllocatorWbt;

class AllocatorService
{
    friend class Singleton<AllocatorService>;

public:
    AllocatorService(void) = default;
    virtual ~AllocatorService(void) = default;

    void RegisterAllocator(std::string arrayName, IBlockAllocator* iBlockAllocator);
    void RegisterAllocator(std::string arrayName, IWBStripeAllocator* iWBStripeAllocator);
    void RegisterAllocator(std::string arrayName, IWBStripeCtx* iWBStripeCtx);
    void RegisterAllocator(std::string arrayName, ISegmentCtx* iSegmentCtx);
    void RegisterAllocator(std::string arrayName, IRebuildCtx* iRebuildCtx);
    void RegisterAllocator(std::string arrayName, IAllocatorCtx* iAllocatorCtx);
    void RegisterAllocator(std::string arrayName, IAllocatorWbt* iAllocatorWbt);
    void UnregisterAllocator(std::string arrayName);

    virtual IBlockAllocator* GetIBlockAllocator(std::string arrayName);
    virtual IWBStripeAllocator* GetIWBStripeAllocator(std::string arrayName);
    IWBStripeCtx* GetIWBStripeCtx(std::string arrayName);
    ISegmentCtx* GetISegmentCtx(std::string arrayName);
    IRebuildCtx* GetIRebuildCtx(std::string arrayName);
    IAllocatorCtx* GetIAllocatorCtx(std::string arrayName);
    IAllocatorWbt* GetIAllocatorWbt(std::string arrayName);

private:
    AllocatorInterfaceContainer<IBlockAllocator> iBlockAllocator;
    AllocatorInterfaceContainer<IWBStripeAllocator> iWBStripeAllocator;
    AllocatorInterfaceContainer<IWBStripeCtx> iWBStripeCtx;
    AllocatorInterfaceContainer<ISegmentCtx> iSegmentCtx;
    AllocatorInterfaceContainer<IRebuildCtx> iRebuildCtx;
    AllocatorInterfaceContainer<IAllocatorCtx> iAllocatorCtx;
    AllocatorInterfaceContainer<IAllocatorWbt> iAllocatorWbt;
};

using AllocatorServiceSingleton = Singleton<AllocatorService>;

} // namespace pos

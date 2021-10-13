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

#include <cstdlib>

#include "assert.h"
#include "src/spdk_wrapper/accel_engine_api.h"
#include "src/include/memory.h"

namespace pos
{
using AsyncMemCpyCallbackPointer = void(void*);
class MetaFsMemLib
{
public:
    MetaFsMemLib(void)
    {
    }
    ~MetaFsMemLib(void)
    {
        Finalize();
    }

    static void
    Init(void)
    {
#if (1 == METAFS_INTEL_IOAT_EN)
        dmaSupported = AccelEngineApi::IsIoatEnable();
#else
        dmaSupported = false;
#endif
        cleanPage = pos::Memory<pos::BLOCK_SIZE>::Alloc(1);
        assert(cleanPage != nullptr); // please check hugepage preallocation
        memset(cleanPage, 0x0, pos::BLOCK_SIZE);
    }

    static void
    Finalize(void)
    {
        if (cleanPage)
        {
            pos::Memory<pos::BLOCK_SIZE>::Free(cleanPage);
            cleanPage = nullptr;
        }
    }

    static void
    EnableResourceUse(void)
    {
        dmaSupported = true;
    }
    static bool
    IsResourceAvailable(void)
    {
        return dmaSupported;
    }

    static void
    MemCpyAsync(void* dst, void* src, size_t nbytes, AsyncMemCpyCallbackPointer callback, void* arg)
    {
        // in order to use this feature, all given memory address must be contiguous and pinned to the application
        assert(dmaSupported == true && nbytes > 0);
        assert((uint64_t)dst > SPDK_ENV_VADDR_BASE && (uint64_t)src > SPDK_ENV_VADDR_BASE);
#if (1 == METAFS_INTEL_IOAT_EN)
        pos::AccelEngineApi::SubmitCopy(dst, src, nbytes, callback, arg);
#endif
    }
    static void
    MemSetZero(void* addr, size_t nbytes, AsyncMemCpyCallbackPointer callback, void* arg)
    {
        assert(dmaSupported == true && nbytes > 0);
        assert((uint64_t)addr > SPDK_ENV_VADDR_BASE);
#if (1 == METAFS_INTEL_IOAT_EN)
        uint8_t* currAddr = (uint8_t*)addr;
        uint64_t currBytes;
        while (nbytes > 0)
        {
            if (nbytes >= pos::PageSize)
            {
                currBytes = pos::PageSize;
            }
            else
            {
                currBytes = nbytes;
            }
            pos::AccelEngineApi::SubmitCopy(currAddr, cleanPage, currBytes, callback, arg);
            nbytes -= currBytes;
            currAddr = currAddr + currBytes;
        }
#endif
    }

private:
    static bool dmaSupported;
    static void* cleanPage;
    // SPDK I/O AT is only applicable to pinned contiguous memory space like hugepage memory
    static const uint64_t SPDK_ENV_VADDR_BASE = 0x200000000000;
};
} // namespace pos

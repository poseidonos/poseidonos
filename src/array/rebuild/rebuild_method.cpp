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

#include "rebuild_method.h"
#include "src/resource_manager/memory_manager.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/logger/logger.h"

namespace pos
{
RebuildMethod::RebuildMethod(uint32_t srcCnt, uint32_t dstCnt, MemoryManager* mm)
: mm(mm)
{
    this->srcSize = (uint64_t)srcCnt * unitSize;
    this->dstSize = (uint64_t)dstCnt * unitSize;
}

RebuildMethod::~RebuildMethod(void)
{
    if (srcBuffer != nullptr)
    {
        if (srcBuffer->IsFull() == false)
        {
            POS_TRACE_ERROR(EID(REBUILD_BUFFER_WARN),
                "Some buffers in srcBuffer were not returned but deleted.");
        }
        mm->DeleteBufferPool(srcBuffer);
        srcBuffer = nullptr;
    }
    if (dstBuffer != nullptr)
    {
        if (dstBuffer->IsFull() == false)
        {
            POS_TRACE_ERROR(EID(REBUILD_BUFFER_WARN),
                "Some buffers in dstBuffer were not returned but deleted.");
        }
        mm->DeleteBufferPool(dstBuffer);
        dstBuffer = nullptr;
    }
}

bool
RebuildMethod::Init(string owner)
{
    unique_lock<mutex> lock(mtx);
    if (isInitialized == true)
    {
        return true;
    }

    this->owner = owner;
    POS_TRACE_INFO(EID(REBUILD_METHOD_INIT),
            "owner:{}, srcSize:{}, dstSize:{}", owner, srcSize, dstSize);
    uint32_t numa = AffinityManagerSingleton::Instance()->GetNumaIdFromCurrentThread();
    BufferInfo srcInfo = {
        .owner = owner,
        .size = srcSize,
        .count = bufCnt};
    srcBuffer = mm->CreateBufferPool(srcInfo, numa);
    if (srcBuffer == nullptr)
    {
        return false;
    }
    BufferInfo dstInfo = {
        .owner = owner,
        .size = dstSize,
        .count = bufCnt};
    dstBuffer = mm->CreateBufferPool(dstInfo, numa);
    if (dstBuffer == nullptr)
    {
        mm->DeleteBufferPool(srcBuffer);
        srcBuffer = nullptr;
        return false;
    }
    POS_TRACE_INFO(EID(REBUILD_BUFFER_INIT_OK),
            "srcSize:{}, dstSize:{}", srcSize, dstSize);

    isInitialized = true;
    return true;
}
} // namespace pos

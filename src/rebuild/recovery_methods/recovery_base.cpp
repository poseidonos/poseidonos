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

#include "recovery_base.h"
#include "src/resource_manager/memory_manager.h"
#include "src/logger/logger.h"

namespace pos
{
RecoveryBase::RecoveryBase(uint64_t srcSize, uint64_t destSize, uint32_t bufCnt)
: srcSize(srcSize), destSize(destSize), bufCnt(bufCnt)
{
}

RecoveryBase::~RecoveryBase(void) 
{
    if (srcBuffer != nullptr)
    {
        if (srcBuffer->IsFull() == false)
        {
            POS_TRACE_ERROR(EID(REBUILD_DEBUG_MSG),
                "Some buffers in srcBuffer were not returned but deleted.");
        }
        mm->DeleteBufferPool(srcBuffer);
        srcBuffer = nullptr;
    }
    if (destBuffer != nullptr)
    {
        if (destBuffer->IsFull() == false)
        {
            POS_TRACE_ERROR(EID(REBUILD_DEBUG_MSG),
                "Some buffers in destBuffer were not returned but deleted.");
        }
        mm->DeleteBufferPool(destBuffer);
        destBuffer = nullptr;
    }
}

bool
RecoveryBase::Init(MemoryManager* mm, string owner)
{
    BufferInfo srcInfo = {
        .owner = owner,
        .size = srcSize,
        .count = bufCnt};
    srcBuffer = mm->CreateBufferPool(srcInfo);
    if (srcBuffer == nullptr)
    {
        return false;
    }

    BufferInfo destInfo = {
        .owner = owner,
        .size = destSize,
        .count = bufCnt};
    destBuffer = mm->CreateBufferPool(destInfo);
    if (destBuffer == nullptr)
    {
        mm->DeleteBufferPool(srcBuffer);
        srcBuffer = nullptr;
        return false;
    }

    this->mm = mm;
    return true;
}

BufferPool*
RecoveryBase::GetDestBuffer(void)
{
    return destBuffer;
}

} // namespace pos

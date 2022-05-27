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

#include "src/gc/copier_meta.h"

#include "src/include/meta_const.h"
#include "src/logger/logger.h"
#include "src/resource_manager/buffer_pool.h"

#include <string>

namespace pos
{

CopierMeta::CopierMeta(IArrayInfo* array)
: CopierMeta(array, array->GetSizeInfo(PartitionType::USER_DATA),
    new BitMapMutex(GC_VICTIM_SEGMENT_COUNT), new GcStripeManager(array),
    nullptr, nullptr)
{
}

CopierMeta::CopierMeta(IArrayInfo* array, const PartitionLogicalSize* udSize,
                       BitMapMutex* inputInUseBitmap, GcStripeManager* inputGcStripeManager,
                       std::vector<std::vector<VictimStripe*>>* inputVictimStripes,
                       std::vector<BufferPool*>* inputGcBufferPool,
                       MemoryManager* memoryManager)
: inUseBitmap(inputInUseBitmap),
  gcStripeManager(inputGcStripeManager),
  arrayName(array->GetName()),
  arrayIndex(array->GetIndex()),
  victimStripes(inputVictimStripes),
  gcBufferPool(inputGcBufferPool),
  memoryManager(memoryManager)
{
    stripesPerSegment = udSize->stripesPerSegment;
    blksPerStripe = udSize->blksPerStripe;

    if (nullptr == inputVictimStripes)
    {
        _CreateVictimStripes(array);
    }
    if (nullptr == inputGcBufferPool)
    {
        _CreateBufferPool(array->GetSizeInfo(PartitionType::USER_DATA)->chunksPerStripe, CHUNK_SIZE);
    }
    InitProgressCount();
}


CopierMeta::~CopierMeta(void)
{
    for (uint32_t index = 0; index < GC_BUFFER_COUNT; index++)
    {
        if (nullptr != (*gcBufferPool)[index])
        {
            memoryManager->DeleteBufferPool((*gcBufferPool)[index]);
        }
    }
    if (nullptr != gcBufferPool)
    {
        delete gcBufferPool;
    }

    for (uint32_t stripeIndex = 0; stripeIndex < GC_VICTIM_SEGMENT_COUNT; stripeIndex++)
    {
        for (uint32_t i = 0 ; i < stripesPerSegment; i++)
        {
            if (nullptr != (*victimStripes)[stripeIndex][i])
            {
                delete (*victimStripes)[stripeIndex][i];
            }
        }
    }
    if (nullptr != victimStripes)
    {
        delete victimStripes;
    }

    if (nullptr != gcStripeManager)
    {
        delete gcStripeManager;
    }
    if (nullptr != inUseBitmap)
    {
        delete inUseBitmap;
    }
}

void*
CopierMeta::GetBuffer(StripeId stripeId)
{
    return (*gcBufferPool)[stripeId % GC_BUFFER_COUNT]->TryGetBuffer();
}

void
CopierMeta::ReturnBuffer(StripeId stripeId, void* buffer)
{
    (*gcBufferPool)[stripeId % GC_BUFFER_COUNT]->ReturnBuffer(buffer);
}

void
CopierMeta::SetStartCopyStripes(void)
{
    requestStripeCount++;
}

void
CopierMeta::SetStartCopyBlks(uint32_t blocks)
{
    requestBlockCount.fetch_add(blocks);
}

void
CopierMeta::SetDoneCopyBlks(uint32_t blocks)
{
    doneBlockCount.fetch_add(blocks);
}

uint32_t
CopierMeta::GetStartCopyBlks(void)
{
    return requestBlockCount;
}

uint32_t
CopierMeta::GetDoneCopyBlks(void)
{
    return doneBlockCount;
}

void
CopierMeta::InitProgressCount(void)
{
    requestStripeCount = 0;
    requestBlockCount = 0;
    doneBlockCount = 0;
}

uint32_t
CopierMeta::SetInUseBitmap(void)
{
    inUseBitmap->SetBit((uint64_t)victimSegmentIndex);
    uint32_t retVal = victimSegmentIndex;
    victimSegmentIndex++;
    victimSegmentIndex = victimSegmentIndex % GC_VICTIM_SEGMENT_COUNT;

    return retVal;
}

bool
CopierMeta::IsSynchronized(void)
{
    bool ret = inUseBitmap->IsSetBit((uint64_t)victimSegmentIndex);

    return !ret;
}

bool
CopierMeta::IsAllVictimSegmentCopyDone(void)
{
    bool ret = false;
    for (uint32_t index = 0; index < GC_VICTIM_SEGMENT_COUNT; index++)
    {
        ret = inUseBitmap->IsSetBit((uint64_t)index);
        if (ret == true)
        {
            IsCopyDone();
            return false;
        }
    }
    return true;
}

bool
CopierMeta::IsCopyDone(void)
{
    if (STRIPES_PER_SEGMENT != requestStripeCount)
    {
        return false;
    }

    if (requestBlockCount != doneBlockCount)
    {
        return false;
    }
    InitProgressCount();

    bool ret = inUseBitmap->IsSetBit((uint64_t)copyIndex);
    if (ret == true)
    {
        inUseBitmap->ClearBit(copyIndex);
    }
    else
    {
        assert(false);
    }
    copyIndex++;
    copyIndex = copyIndex % GC_VICTIM_SEGMENT_COUNT;

    copyLock.clear();
    return true;
}

bool
CopierMeta::IsReadytoCopy(uint32_t index)
{
    if (index == copyIndex)
    {
        if (copyLock.test_and_set() == false)
        {
            return true;
        }
    }
    return false;
}

uint32_t
CopierMeta::GetStripePerSegment(void)
{
    return stripesPerSegment;
}

uint32_t
CopierMeta::GetBlksPerStripe(void)
{
    return blksPerStripe;
}

VictimStripe*
CopierMeta::GetVictimStripe(uint32_t victimSegmentIndex, uint32_t stripeOffset)
{
    return (*victimStripes)[victimSegmentIndex][stripeOffset];
}

GcStripeManager*
CopierMeta::GetGcStripeManager(void)
{
    return gcStripeManager;
}

void
CopierMeta::_CreateBufferPool(uint64_t maxBufferCount, uint32_t bufferSize)
{
    gcBufferPool = new std::vector<BufferPool*>;
    for (uint32_t index = 0; index < GC_BUFFER_COUNT; index++)
    {
        BufferInfo info = {
            .owner = typeid(this).name() + to_string(index),
            .size = bufferSize,
            .count = maxBufferCount
        };
        gcBufferPool->push_back(memoryManager->CreateBufferPool(info));
    }
}

void
CopierMeta::_CreateVictimStripes(IArrayInfo* array)
{
    victimStripes = new std::vector<std::vector<VictimStripe*>>;
    victimStripes->resize(GC_VICTIM_SEGMENT_COUNT);
    for (uint32_t stripeIndex = 0; stripeIndex < GC_VICTIM_SEGMENT_COUNT; stripeIndex++)
    {
        for (uint32_t i = 0 ; i < stripesPerSegment; i++)
        {
            (*victimStripes)[stripeIndex].push_back(new VictimStripe(array));
        }
    }
}

std::string
CopierMeta::GetArrayName(void)
{
    return arrayName;
}

unsigned int
CopierMeta::GetArrayIndex(void)
{
    return arrayIndex;
}

} // namespace pos

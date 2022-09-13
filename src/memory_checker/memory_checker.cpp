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

#include "memory_checker.h"

#include <cassert>
#include <cstring>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
std::atomic<bool> MemoryChecker::enable;
std::atomic<bool> FreeListInfo::enableStackTrace;
thread_local bool inMemoryCheckerContext = false;
std::map<uint64_t, FreeListInfo*> MemoryChecker::freeListMap;
std::unordered_map<uint64_t, size_t> MemoryChecker::allocateMap;
thread_local bool MemoryChecker::inMemoryCheckerContext;
std::mutex MemoryChecker::mapMutex;
const uint32_t MemoryChecker::PADDING_BYTE = 8;

FreeListInfo::FreeListInfo(size_t inputSize):
size(inputSize)
{
    if (enableStackTrace)
    {
        backtrace(dumpStack, FreeListInfo::MAX_DUMP_STACK_COUNT);
    }
}

FreeListInfo::FreeListInfo(size_t inputSize, void* inputDumpStack[]):
size(inputSize)
{
    if (enableStackTrace)
    {
        for (uint32_t index = 0; index < MAX_DUMP_STACK_COUNT; index++)
        {
            dumpStack[index] = inputDumpStack[index];
        }
    }
}

void
FreeListInfo::PrintDumpStack(void)
{
    if (enableStackTrace)
    {
        backtrace_symbols_fd(dumpStack,
            FreeListInfo::MAX_DUMP_STACK_COUNT, STDERR_FILENO);
    }
}
// We erase from free list for given address.
// free list has multiple chunks, and given address can overlap the ranges of these chunks.
// allcator (like tc malloc) manages its free chunks as merging or split them.
// So, we consider partial allocation case from free list.
void
MemoryChecker::EraseFromFreeList(uint64_t address, std::size_t size)
{
    if (freeListMap.empty() == true)
    {
        return;
    }
    auto startIterator = freeListMap.upper_bound(address);
    if (startIterator != freeListMap.begin())
    {
        startIterator = std::prev(startIterator);
    }

    auto endIterator = freeListMap.lower_bound(address + size);
    if (endIterator == freeListMap.begin())
    {
        return;
    }

    auto lastIterator = std::prev(endIterator);

    uint64_t startSplitAddress = 0, startSplitSize = 0;
    uint64_t endSplitAddress = 0, endSplitSize = 0;
    FreeListInfo* deletedPtr = nullptr;

    for (auto iterator = startIterator; iterator != endIterator;)
    {
        if (iterator == freeListMap.end())
        {
            break;
        }
        if (iterator == startIterator)
        {
            if (iterator->first + iterator->second->size <= address)
            {
                iterator = std::next(iterator);
                continue;
            }
            else
            {
                startSplitAddress = iterator->first;
                startSplitSize = address - iterator->first;
            }
        }
        if (iterator == lastIterator)
        {
            if (iterator->first + iterator->second->size > address + size)
            {
                endSplitAddress = address + size;
                endSplitSize = iterator->first + iterator->second->size - (address + size);
            }
        }
        deletedPtr = iterator->second;
        iterator = freeListMap.erase(iterator);
    }
    if (startSplitSize != 0)
    {
        FreeListInfo* freeListInfo = new FreeListInfo(startSplitSize, deletedPtr->dumpStack);
        freeListMap[startSplitAddress] = freeListInfo;
    }
    if (endSplitAddress != 0)
    {
        FreeListInfo* freeListInfo = new FreeListInfo(endSplitSize, deletedPtr->dumpStack);
        freeListMap[endSplitAddress] = freeListInfo;
    }
    if (deletedPtr != nullptr)
    {
        delete deletedPtr;
    }
}

// If any free trial to free list ranges is detected, we assert.
void
MemoryChecker::_CheckDoubleFree(uint64_t baseAddress)
{
    if (freeListMap.empty() == true)
    {
        return;
    }
    // We first check base address is completely equal to base addresses in free list.
    if (freeListMap.find(baseAddress) != freeListMap.end())
    {
        freeListMap[baseAddress]->PrintDumpStack();
        POS_TRACE_ERROR(EID(DEBUG_MEMORY_CHECK_DOUBLE_FREE),
            "double free {}", baseAddress);
        assert(0);
    }

    // We check, if range of free list includes given baseAddress.
    auto iterator = freeListMap.upper_bound(baseAddress);
    if (iterator == freeListMap.begin())
    {
        return;
    }

    iterator = std::prev(iterator);

    if (iterator->first + iterator->second->size > baseAddress)
    {
        freeListMap[baseAddress]->PrintDumpStack();
        POS_TRACE_ERROR(EID(DEBUG_MEMORY_CHECK_DOUBLE_FREE),
            "double free {} {} {}", iterator->first, iterator->second->size, baseAddress);
        assert(0);
    }
}

void*
MemoryChecker::_TrackNew(std::size_t size)
{
    inMemoryCheckerContext = true;
    // padding pattern is its allocated address.
    void* ptr = std::malloc(PADDING_BYTE + size);

    // inMemoryCheckerContext prevent map or unordered_map from calling this function again.
    // If the ptr is not added to the allocate map, we just abandon that case.

    uint64_t paddingPattern = reinterpret_cast<uint64_t>(ptr);
    uint64_t key = reinterpret_cast<uint64_t>(ptr);

    *reinterpret_cast<uint64_t*>(static_cast<uint8_t*>(ptr) + size) = paddingPattern;
    {
        std::lock_guard<std::mutex> guard(mapMutex);
        EraseFromFreeList(key, size + PADDING_BYTE);
        allocateMap[key] = size + PADDING_BYTE;
    }
    inMemoryCheckerContext = false;

    return (void*)((uint8_t*)ptr);
}

void
MemoryChecker::_TrackDelete(void* ptr)
{
    inMemoryCheckerContext = true;
    uint64_t key = reinterpret_cast<uint64_t>(ptr);

    {
        std::lock_guard<std::mutex> guard(mapMutex);
        _CheckDoubleFree(key);

        if (allocateMap.find(key) != allocateMap.end())
        {
            size_t size = allocateMap[key];

            uint64_t patternSuffix = *reinterpret_cast<uint64_t*>(static_cast<uint8_t*>(ptr) + size - PADDING_BYTE);
            if (patternSuffix != key)
            {
                freeListMap[key]->PrintDumpStack();
                POS_TRACE_ERROR(EID(DEBUG_MEMORY_CHECK_INVALID_ACCESS),
                    "invalid access base : {} size : {}", key, (size - PADDING_BYTE));
                assert(0);
            }
            FreeListInfo* freeListInfo = new FreeListInfo(size);
            freeListMap[key] = freeListInfo;
            allocateMap.erase(key);

            // If ptr is Invalid pointer (unknown by Kernel) It will incur segmentation failure in this point.
            memset(ptr, 0x0, size);
        }
    }
    // this case, main fw already allocate the memory so, we don't have that information.
    inMemoryCheckerContext = false;
    std::free(ptr);
}

void*
MemoryChecker::New(std::size_t size)
{
    if (likely(enable == false || inMemoryCheckerContext == true))
    {
        return std::malloc(size);
    }
    return _TrackNew(size);
}

void
MemoryChecker::Delete(void* ptr)
{
    if (likely(enable == false || inMemoryCheckerContext == true))
    {
        std::free(ptr);
    }
    else
    {
        _TrackDelete(ptr);
    }
}

void
pos::MemoryChecker::EnableStackTrace(bool flag)
{
    FreeListInfo::enableStackTrace = flag;
}

void
pos::MemoryChecker::Enable(bool flag)
{
    enable = flag;
}

}; // namespace pos

using namespace pos;
void*
operator new(std::size_t size)
{
    return MemoryChecker::New(size);
}

void
operator delete(void* ptr)
{
    MemoryChecker::Delete(ptr);
}

void*
operator new[](std::size_t size)
{
    return MemoryChecker::New(size);
}

void
operator delete[](void* ptr)
{
    MemoryChecker::Delete(ptr);
}

void
operator delete(void* ptr, std::size_t size)
{
    MemoryChecker::Delete(ptr);
}

void
operator delete[](void* ptr, std::size_t size)
{
    MemoryChecker::Delete(ptr);
}

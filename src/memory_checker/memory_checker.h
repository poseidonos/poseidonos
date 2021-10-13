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

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <mutex>
#include <unordered_map>


namespace pos
{

class FreeListInfo
{
public:
    explicit FreeListInfo(size_t inputSize);
    FreeListInfo(size_t inputSize, void* inputDumpStack[]);
    void PrintDumpStack(void);
    size_t size;
    static const uint32_t MAX_DUMP_STACK_COUNT = 10;
    static std::atomic<bool> enableStackTrace;
    void* dumpStack[MAX_DUMP_STACK_COUNT];
};
class MemoryChecker
{
public:
    static void* New(std::size_t size);
    static void Delete(void* ptr);
    static void Enable(bool flag);
    // previous owner can be traced with this option.
    static void EnableStackTrace(bool flag);
    static void EraseFromFreeList(uint64_t address, std::size_t size);

private:
    static void _CheckDoubleFree(uint64_t baseAddress);
    static void* _TrackNew(std::size_t size);
    static void _TrackDelete(void* ptr);

    static std::unordered_map<uint64_t, size_t> allocateMap;
    static std::map<uint64_t, FreeListInfo*> freeListMap;
    static std::atomic<bool> enable;
    static std::mutex mapMutex;

    // this prevent recursive call by internal memory allocation
    static thread_local bool inMemoryCheckerContext;
    static const uint32_t PADDING_BYTE;
};
} // namespace pos

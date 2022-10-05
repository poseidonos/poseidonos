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

#ifndef IOAT_API_H_
#define IOAT_API_H_

#include <atomic>
#include <vector>

#include "rte_config.h"
#include "src/spdk_wrapper/event_framework_api.h"

struct spdk_io_channel;

namespace pos
{
using IoatCb = void (*)(void*);
class AffinityManager;

struct IoatArgument
{
    IoatArgument(void* dst, void* src, uint64_t bytes,
        IoatCb cbFunction, void* cbArgument)
    : dst(dst),
      src(src),
      bytes(bytes),
      cbFunction(cbFunction),
      cbArgument(cbArgument)
    {
    }
    void* dst;
    void* src;
    uint64_t bytes;
    IoatCb cbFunction;
    void* cbArgument;
};
class AccelEngineApi
{
public:
    static void Initialize(void);
    static void SubmitCopy(void* dst,
        void* src,
        uint64_t bytes,
        IoatCb cbFunction,
        void* cbArgument);
    static void Finalize(EventFrameworkApi* eventFrameworkApi = nullptr);
    static bool IsIoatEnable(void);
    static int GetIoatReactorByIndex(uint32_t index);
    static int GetReactorByIndex(uint32_t index);
    static uint32_t GetIoatReactorCount(void);
    static uint32_t GetReactorCount(void);
    static void SetReactorCount(uint32_t count);
    static uint32_t GetIoatReactorCountPerNode(uint32_t node);

private:
    static std::atomic<bool> enabled;
    static std::atomic<bool> initialized;
    static std::atomic<bool> finalized;
    static std::atomic<uint32_t> ioatReactorCount;
    static std::atomic<uint32_t> reactorCount;
    static std::atomic<uint32_t> ioatReactorCountPerNode[RTE_MAX_NUMA_NODES];
    static uint32_t detected_numa_count;
    static thread_local spdk_io_channel* spdkChannel;

    static void _HandleInitialize(void* arg1);
    static void _HandleCopy(void* arg1, void* arg2);
    static void _HandleFinalize(void* arg1);
    static bool _IsChannelValid(void);
    static void _SetChannel(void);
    static void _PutChannel(void);
    static void _SetIoat(void);
    static const int INVALID_REACTOR = -1;
    static uint32_t ioatReactorArray[RTE_MAX_LCORE];
    static uint32_t reactorArray[RTE_MAX_LCORE];
    static thread_local uint32_t requestCount;
};
} // namespace pos

#endif // IOAT_API_H_

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

#include <atomic>
#include <string>
#include <vector>

#include "src/network/nvmf_target_spdk.hpp"
using namespace std;

namespace pos
{
enum NvmfCallbackStatus
{
    SUCCESS = 0,
    FAILED = -2,
    PARTIAL_FAILED = -1
};

class NvmfTarget
{
public:
    NvmfTarget(void);
    ~NvmfTarget(void);

    bool IsTargetExist(void);
    bool CreatePosBdev(const string& bdevName, uint32_t id, uint64_t volumeSizeInMb,
        uint32_t blockSize, bool volumeTypeInMem, const string& arrayName);
    bool DeletePosBdev(const string& bdevName);

    bool AttachNamespace(const string& nqn, const string& bdevName,
        PosNvmfEventDoneCallback_t cb, void* cbArg);
    bool AttachNamespace(const string& nqn, const string& bdevName, uint32_t nsid,
        PosNvmfEventDoneCallback_t cb, void* cbArg);
    bool DetachNamespace(const string& nqn, uint32_t nsid,
        PosNvmfEventDoneCallback_t cb, void* cbArg);
    bool DetachNamespaceAll(const string& nqn, PosNvmfEventDoneCallback_t cb,
        void* cbArg);

    uint32_t GetSubsystemNsCnt(struct spdk_nvmf_subsystem* subsystem);
    struct spdk_nvmf_subsystem* AllocateSubsystem(void);
    struct spdk_nvmf_ns* GetNamespace(struct spdk_nvmf_subsystem* subsystem,
        const string& bdevName);

    void SetVolumeQos(const string& bdevName, uint64_t maxIops, uint64_t maxBw);
    static void QosEnableDone(void* cbArg, int status);

    string GetBdevName(uint32_t id, string arrayName);
    string GetVolumeNqn(struct spdk_nvmf_subsystem* subsystem);
    uint32_t GetVolumeNqnId(const string& subnqn);
    spdk_nvmf_subsystem* FindSubsystem(const string& nqn);
    vector<string> GetHostNqn(string subnqn);
    bool TryToAttachNamespace(const string& nqn, int volId, string& arrayName);
    bool CheckSubsystemExistance(void);
    bool CheckVolumeAttached(int volId, string arrayName);

private:
    static struct NvmfTargetCallbacks nvmfCallbacks;
    uint32_t nrVolumePerSubsystem = 1;
    static const char* BDEV_NAME_PREFIX;
    static atomic<int> attachedNsid;

    struct EventContext* _CreateEventContext(PosNvmfEventDoneCallback_t callback,
        void* userArg, void* eventArg1, void* eventArg2);
    static void _DetachNamespaceWithPause(void* arg1, void* arg2);
    static void _AttachNamespaceWithPause(void* arg1, void* arg2);
    static void _DetachNamespaceAllWithPause(void* arg1, void* arg2);
    static void _AttachDone(void* cbArg, int status);
    static void _TryAttachHandler(void* arg1, void* arg2);
};

} // namespace pos

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

#include "src/network/nvmf_target.h"

namespace pos
{
class NvmfTargetSpy : public NvmfTarget
{
public:
    using NvmfTarget::NvmfTarget;
    NvmfTargetSpy(SpdkCaller* spdkCaller, bool feQosEnable, EventFrameworkApi* eventFrameworkApi)
    : NvmfTarget(spdkCaller, feQosEnable, eventFrameworkApi)
    {
    }

    void
    AttachNamespacePauseDone(struct spdk_nvmf_subsystem* subsystem, void* arg, int status)
    {
        nvmfCallbacks.attachNamespacePauseDone(subsystem, arg, status);
    }

    void
    DetachNamespacePauseDone(struct spdk_nvmf_subsystem* subsystem, void* arg, int status)
    {
        nvmfCallbacks.detachNamespacePauseDone(subsystem, arg, status);
    }

    void
    DetachNamespaceAllPauseDone(struct spdk_nvmf_subsystem* subsystem, void* arg, int status)
    {
        nvmfCallbacks.detachNamespaceAllPauseDone(subsystem, arg, status);
    }

    void
    AttachNamespaceWithPause(void* arg1, void* arg2,
        EventFrameworkApi* eventFrameworkApi, SpdkNvmfCaller* spdkNvmfCaller)
    {
        _AttachNamespaceWithPause(arg1, arg2, eventFrameworkApi, spdkNvmfCaller);
    }

    void
    DetachNamespaceWithPause(void* arg1, void* arg2,
        EventFrameworkApi* eventFrameworkApi, SpdkNvmfCaller* spdkNvmfCaller)
    {
        _DetachNamespaceWithPause(arg1, arg2, eventFrameworkApi, spdkNvmfCaller);
    }

    void
    DetachNamespaceAllWithPause(void* arg1, void* arg2,
        EventFrameworkApi* eventFrameworkApi, SpdkNvmfCaller* spdkNvmfCaller)
    {
        _DetachNamespaceAllWithPause(arg1, arg2, eventFrameworkApi, spdkNvmfCaller);
    }
    
    void
    DeletePosBdevAllHandler(void* arg1, SpdkCaller* spdkCaller, SpdkBdevCaller* spdkBdevCaller)
    {
        NvmfTarget::_DeletePosBdevAllHandler(arg1, spdkCaller, spdkBdevCaller);
    }
};
} // namespace pos

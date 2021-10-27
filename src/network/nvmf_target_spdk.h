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

#include <unistd.h>

#include <functional>
#include <iostream>

#include "spdk/nvme.h"
#include "spdk/nvmf.h"
#include "spdk/stdinc.h"
#include "spdk/string.h"
#include "src/spdk_wrapper/caller/spdk_caller.h"

typedef void (*PosNvmfEventDoneCallback_t)(void* cb_arg, int status);

namespace pos
{
struct EventContext
{
    PosNvmfEventDoneCallback_t userCallback;
    void* userArg;
    void* eventArg1;
    void* eventArg2;
};

struct NvmfTargetCallbacks
{
    struct EventContext* (*allocEventContext)(PosNvmfEventDoneCallback_t callback, void* userArg);
    void (*freeEventContext)(struct EventContext* e);

    void (*createPosBdevDone)(void* arg, int status);
    void (*deletePosBdevDone)(void* arg, int status);

    void (*subsystemResumeDone)(struct spdk_nvmf_subsystem* subsystem, void* arg, int status);

    void (*attachNamespacePauseDone)(struct spdk_nvmf_subsystem* subsystem, void* arg, int status);
    void (*attachNamespaceResumeDone)(struct spdk_nvmf_subsystem* subsystem, void* arg, int status);
    void (*detachNamespacePauseDone)(struct spdk_nvmf_subsystem* subsystem, void* arg, int status);
    void (*detachNamespaceResumeDone)(struct spdk_nvmf_subsystem* subsystem, void* arg, int status);
    void (*detachNamespaceAllPauseDone)(struct spdk_nvmf_subsystem* subsystem, void* arg, int status);
    void (*detachNamespaceAllResumeDone)(struct spdk_nvmf_subsystem* subsystem, void* arg, int status);

    void (*attachListenerPauseDone)(struct spdk_nvmf_subsystem* subsystem, void* arg, int status);
    void (*targetListenDone)(void* arg, int status);
    void (*detachListenerPauseDone)(struct spdk_nvmf_subsystem* subsystem, void* arg, int status);
};

void InitNvmfCallbacks(struct NvmfTargetCallbacks* nvmfCallbacks);
struct EventContext* AllocEventContext(PosNvmfEventDoneCallback_t callback, void* userArg);
void FreeEventContext(struct EventContext* e);
void ActivateSubsystem(void* arg1);

} // namespace pos

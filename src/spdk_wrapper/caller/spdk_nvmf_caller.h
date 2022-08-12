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

#include "spdk/nvmf.h"
#include "spdk/pos_nvmf.h"

namespace pos
{
class SpdkNvmfCaller
{
public:
    SpdkNvmfCaller(void);
    virtual ~SpdkNvmfCaller(void);
    virtual struct spdk_nvmf_tgt* SpdkNvmfGetTgt(const char* name);
    virtual struct spdk_nvmf_subsystem* SpdkNvmfTgtFindSubsystem(struct spdk_nvmf_tgt* tgt, const char* subnqn);
    virtual enum spdk_nvmf_subtype SpdkNvmfSubsystemGetType(struct spdk_nvmf_subsystem* subsystem);
    virtual const char* SpdkNvmfSubsystemGetNqn(const struct spdk_nvmf_subsystem* subsystem);
    virtual struct spdk_nvmf_subsystem* SpdkNvmfSubsystemGetFirst(struct spdk_nvmf_tgt* tgt);
    virtual struct spdk_nvmf_subsystem* SpdkNvmfSubsystemGetNext(struct spdk_nvmf_subsystem* subsystem);
    virtual struct spdk_nvmf_ns* SpdkNvmfSubsystemGetNs(struct spdk_nvmf_subsystem* subsystem, uint32_t nsid);
    virtual struct spdk_nvmf_ns* SpdkNvmfSubsystemGetFirstNs(struct spdk_nvmf_subsystem* subsystem);
    virtual struct spdk_nvmf_ns* SpdkNvmfSubsystemGetNextNs(struct spdk_nvmf_subsystem* subsystem,
        struct spdk_nvmf_ns* prevNs);
    virtual uint32_t SpdkNvmfSubsystemAddNs(struct spdk_nvmf_subsystem* subsystem, const char* bdevName, const struct spdk_nvmf_ns_opts* user_opts, size_t opts_size, const char* ptpl_file);
    virtual int SpdkNvmfSubsystemRemoveNs(struct spdk_nvmf_subsystem* subsystem, uint32_t nsid);
    virtual int SpdkNvmfSubsystemPause(struct spdk_nvmf_subsystem* subsystem, uint32_t nsid, spdk_nvmf_subsystem_state_change_done cbFunc, void* cbArg);
    virtual int SpdkNvmfSubsystemResume(struct spdk_nvmf_subsystem* subsystem, spdk_nvmf_subsystem_state_change_done cbFunc, void* cbArg);
    virtual struct spdk_bdev* SpdkNvmfNsGetBdev(struct spdk_nvmf_ns* ns);
    virtual uint32_t SpdkNvmfNsGetId(const struct spdk_nvmf_ns* ns);
    virtual void SpdkNvmfInitializeNumaAwarePollGroup(void);
    virtual void SpdkNvmfSetUseEventReactor(cpu_set_t eventReactorSet);
};
} // namespace pos

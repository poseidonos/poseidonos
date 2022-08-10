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

#include "src/spdk_wrapper/caller/spdk_nvmf_caller.h"

namespace pos
{
SpdkNvmfCaller::SpdkNvmfCaller(void)
{
}

SpdkNvmfCaller::~SpdkNvmfCaller(void)
{
}

struct spdk_nvmf_tgt*
SpdkNvmfCaller::SpdkNvmfGetTgt(const char* name)
{
    return spdk_nvmf_get_tgt(name);
}

struct spdk_nvmf_subsystem*
SpdkNvmfCaller::SpdkNvmfTgtFindSubsystem(
    struct spdk_nvmf_tgt* tgt,
    const char* subnqn)
{
    return spdk_nvmf_tgt_find_subsystem(tgt, subnqn);
}

enum spdk_nvmf_subtype
SpdkNvmfCaller::SpdkNvmfSubsystemGetType(struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_type(subsystem);
}

const char*
SpdkNvmfCaller::SpdkNvmfSubsystemGetNqn(const struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_nqn(subsystem);
}

struct spdk_nvmf_subsystem*
SpdkNvmfCaller::SpdkNvmfSubsystemGetFirst(struct spdk_nvmf_tgt* tgt)
{
    return spdk_nvmf_subsystem_get_first(tgt);
}

struct spdk_nvmf_subsystem*
SpdkNvmfCaller::SpdkNvmfSubsystemGetNext(struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_next(subsystem);
}

struct spdk_nvmf_ns*
SpdkNvmfCaller::SpdkNvmfSubsystemGetNs(struct spdk_nvmf_subsystem* subsystem, uint32_t nsid)
{
    return spdk_nvmf_subsystem_get_ns(subsystem, nsid);
}

struct spdk_nvmf_ns*
SpdkNvmfCaller::SpdkNvmfSubsystemGetFirstNs(struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_first_ns(subsystem);
}

struct spdk_nvmf_ns*
SpdkNvmfCaller::SpdkNvmfSubsystemGetNextNs(
    struct spdk_nvmf_subsystem* subsystem,
    struct spdk_nvmf_ns* prevNs)
{
    return spdk_nvmf_subsystem_get_next_ns(subsystem, prevNs);
}

uint32_t
SpdkNvmfCaller::SpdkNvmfSubsystemAddNs(struct spdk_nvmf_subsystem* subsystem, const char* bdevName,
    const struct spdk_nvmf_ns_opts* user_opts, size_t opts_size, const char* ptpl_file)
{
    return spdk_nvmf_subsystem_add_ns_ext(subsystem, bdevName, user_opts, opts_size, ptpl_file);
}

int
SpdkNvmfCaller::SpdkNvmfSubsystemRemoveNs(struct spdk_nvmf_subsystem* subsystem, uint32_t nsid)
{
    return spdk_nvmf_subsystem_remove_ns(subsystem, nsid);
}

int
SpdkNvmfCaller::SpdkNvmfSubsystemPause(
    struct spdk_nvmf_subsystem* subsystem,
    uint32_t nsid,
    spdk_nvmf_subsystem_state_change_done cbFunc,
    void* cbArg)
{
    return spdk_nvmf_subsystem_pause(subsystem, nsid, cbFunc, cbArg);
}

int
SpdkNvmfCaller::SpdkNvmfSubsystemResume(
    struct spdk_nvmf_subsystem* subsystem,
    spdk_nvmf_subsystem_state_change_done cbFunc,
    void* cbArg)
{
    return spdk_nvmf_subsystem_resume(subsystem, cbFunc, cbArg);
}

struct spdk_bdev*
SpdkNvmfCaller::SpdkNvmfNsGetBdev(struct spdk_nvmf_ns* ns)
{
    return spdk_nvmf_ns_get_bdev(ns);
}

uint32_t
SpdkNvmfCaller::SpdkNvmfNsGetId(const struct spdk_nvmf_ns* ns)
{
    return spdk_nvmf_ns_get_id(ns);
}

void
SpdkNvmfCaller::SpdkNvmfInitializeNumaAwarePollGroup(void)
{
    spdk_nvmf_initialize_numa_aware_poll_group();
}

void
SpdkNvmfCaller::SpdkNvmfSetUseEventReactor(cpu_set_t eventReactorSet)
{
    spdk_nvmf_set_use_event_reactor(eventReactorSet);
}

} // namespace pos

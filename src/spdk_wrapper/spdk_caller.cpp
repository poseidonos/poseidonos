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

#include "src/spdk_wrapper/spdk_caller.h"

#include "spdk/bdev.h"
#include "spdk/bdev_module.h"

namespace pos
{
SpdkCaller::SpdkCaller(void)
{
}

SpdkCaller::~SpdkCaller(void)
{
}

struct spdk_nvmf_tgt*
SpdkCaller::SpdkNvmfGetTgt(const char* name)
{
    return spdk_nvmf_get_tgt(name);
}

struct spdk_nvmf_subsystem*
SpdkCaller::SpdkNvmfTgtFindSubsystem(struct spdk_nvmf_tgt* tgt, const char* subnqn)
{
    return spdk_nvmf_tgt_find_subsystem(tgt, subnqn);
}

enum spdk_nvmf_subtype
SpdkCaller::SpdkNvmfSubsystemGetType(struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_type(subsystem);
}

const char*
SpdkCaller::SpdkNvmfSubsystemGetNqn(const struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_nqn(subsystem);
}

struct spdk_nvmf_subsystem*
SpdkCaller::SpdkNvmfSubsystemGetFirst(struct spdk_nvmf_tgt* tgt)
{
    return spdk_nvmf_subsystem_get_first(tgt);
}

struct spdk_nvmf_subsystem*
SpdkCaller::SpdkNvmfSubsystemGetNext(struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_next(subsystem);
}

char*
SpdkCaller::SpdkNvmfSubsystemGetCtrlrHostnqn(struct spdk_nvmf_ctrlr* ctrlr)
{
    return spdk_nvmf_subsystem_get_ctrlr_hostnqn(ctrlr);
}

struct spdk_nvmf_ctrlr*
SpdkCaller::SpdkNvmfSubsystemGetFirstCtrlr(struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_first_ctrlr(subsystem);
}

struct spdk_nvmf_ctrlr*
SpdkCaller::SpdkNvmfSubsystemGetNextCtrlr(struct spdk_nvmf_subsystem* subsystem,
    struct spdk_nvmf_ctrlr* prevCtrlr)
{
    return spdk_nvmf_subsystem_get_next_ctrlr(subsystem, prevCtrlr);
}

struct spdk_nvmf_ns *
SpdkCaller::SpdkNvmfSubsystemGetNs(struct spdk_nvmf_subsystem *subsystem, uint32_t nsid)
{
    return spdk_nvmf_subsystem_get_ns(subsystem, nsid);
}

struct spdk_nvmf_ns*
SpdkCaller::SpdkNvmfSubsystemGetFirstNs(struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_first_ns(subsystem);
}

struct spdk_nvmf_ns*
SpdkCaller::SpdkNvmfSubsystemGetNextNs(struct spdk_nvmf_subsystem* subsystem,
    struct spdk_nvmf_ns* prevNs)
{
    return spdk_nvmf_subsystem_get_next_ns(subsystem, prevNs);
}

uint32_t
SpdkCaller::SpdkNvmfSubsystemAddNs(struct spdk_nvmf_subsystem *subsystem, const char* bdevName,
   const struct spdk_nvmf_ns_opts *user_opts, size_t opts_size, const char *ptpl_file)
{
    return spdk_nvmf_subsystem_add_ns_ext(subsystem, bdevName, user_opts, opts_size, ptpl_file);
}

int
SpdkCaller::SpdkNvmfSubsystemRemoveNs(struct spdk_nvmf_subsystem* subsystem, uint32_t nsid)
{
    return spdk_nvmf_subsystem_remove_ns(subsystem, nsid);
}

int
SpdkCaller::SpdkNvmfSubsystemPause(struct spdk_nvmf_subsystem* subsystem,
    uint32_t nsid,
    spdk_nvmf_subsystem_state_change_done cbFunc, void* cbArg)
{
    return spdk_nvmf_subsystem_pause(subsystem, nsid, cbFunc, cbArg);
}

int
SpdkCaller::SpdkNvmfSubsystemResume(struct spdk_nvmf_subsystem* subsystem,
    spdk_nvmf_subsystem_state_change_done cbFunc, void* cbArg)
{
    return spdk_nvmf_subsystem_resume(subsystem, cbFunc, cbArg);
}

int
SpdkCaller::SpdkNvmfSubsystemSetPauseDirectly(struct spdk_nvmf_subsystem *subsystem)
{
    return spdk_nvmf_subsystem_set_pause_state_directly(subsystem);
}

struct spdk_bdev*
SpdkCaller::SpdkNvmfNsGetBdev(struct spdk_nvmf_ns* ns)
{
    return spdk_nvmf_ns_get_bdev(ns);
}

uint32_t
SpdkCaller::SpdkNvmfNsGetId(const struct spdk_nvmf_ns* ns)
{
    return spdk_nvmf_ns_get_id(ns);
}

uint32_t
SpdkCaller::SpdkNvmfSubsystemGetId(spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_id(subsystem);
}

struct spdk_bdev*
SpdkCaller::SpdkBdevCreatePosDisk(const char* volume_name, uint32_t volumeId,
    const struct spdk_uuid* bdevUuid, uint64_t numBlocks, uint32_t blockSize,
    bool volume_type_in_memory, const char* array_name, uint64_t arrayId)
{
    return spdk_bdev_create_pos_disk(volume_name, volumeId, bdevUuid, numBlocks, blockSize,
        volume_type_in_memory, array_name, arrayId);
}

void
SpdkCaller::SpdkBdevDeletePosDisk(struct spdk_bdev* bdev, pos_bdev_delete_callback cbFunc, void* cbArg)
{
    return spdk_bdev_delete_pos_disk(bdev, cbFunc, cbArg);
}

struct spdk_bdev*
SpdkCaller::SpdkBdevGetByName(const char* bdevName)
{
    return spdk_bdev_get_by_name(bdevName);
}

const char*
SpdkCaller::SpdkBdevGetName(const struct spdk_bdev* bdev)
{
    return spdk_bdev_get_name(bdev);
}

void
SpdkCaller::SpdkBdevSetQosRateLimits(struct spdk_bdev* bdev, uint64_t* limits,
    void (*cbFunc)(void* cbArg, int status), void* cbArg)
{
    return spdk_bdev_set_qos_rate_limits(bdev, limits, cbFunc, cbArg);
}

const char*
SpdkCaller::SpdkGetAttachedSubsystemNqn(const char* bdevName)
{
    return get_attached_subsystem_nqn(bdevName);
}

const struct spdk_uuid*
SpdkCaller::SpdkBdevGetUuid(const struct spdk_bdev* bdev)
{
    return spdk_bdev_get_uuid(bdev);
}

int
SpdkCaller::SpdkUuidFmtLower(char* uuid_str, size_t uuid_str_size, const struct spdk_uuid* uuid)
{
    return spdk_uuid_fmt_lower(uuid_str, uuid_str_size, uuid);
}

int
SpdkCaller::SpdkUuidParse(struct spdk_uuid* uuid, const char* uuid_str)
{
    return spdk_uuid_parse(uuid, uuid_str);
}
} // namespace pos

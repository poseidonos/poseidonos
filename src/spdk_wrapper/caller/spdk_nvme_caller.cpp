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

#include "spdk_nvme_caller.h"

using namespace pos;

SpdkNvmeCaller::SpdkNvmeCaller(void)
{
}

SpdkNvmeCaller::~SpdkNvmeCaller(void)
{
}

uint32_t
SpdkNvmeCaller::SpdkNvmeNsGetId(struct spdk_nvme_ns* ns)
{
    return spdk_nvme_ns_get_id(ns);
}

struct spdk_nvme_ctrlr*
SpdkNvmeCaller::SpdkNvmeNsGetCtrlr(struct spdk_nvme_ns* ns)
{
    return spdk_nvme_ns_get_ctrlr(ns);
}

int
SpdkNvmeCaller::SpdkNvmeNsCmdDatasetManagement(
    struct spdk_nvme_ns* ns,
    struct spdk_nvme_qpair* qpair,
    uint32_t type,
    const struct spdk_nvme_dsm_range* range,
    uint16_t num_ranges,
    spdk_nvme_cmd_cb cb_fn,
    void* cb_arg)
{
    return spdk_nvme_ns_cmd_dataset_management(
        ns, qpair, type, range, num_ranges, cb_fn, cb_arg);
}

int
SpdkNvmeCaller::SpdkNvmeNsCmdRead(
    struct spdk_nvme_ns* ns,
    struct spdk_nvme_qpair* qpair,
    void* buffer,
    uint64_t lba,
    uint32_t lba_count,
    spdk_nvme_cmd_cb cb_fn,
    void* cb_arg,
    uint32_t io_flags)
{
    return spdk_nvme_ns_cmd_read(
        ns, qpair, buffer, lba, lba_count, cb_fn, cb_arg, io_flags);
}

int
SpdkNvmeCaller::SpdkNvmeNsCmdWrite(
    struct spdk_nvme_ns* ns,
    struct spdk_nvme_qpair* qpair,
    void* buffer,
    uint64_t lba,
    uint32_t lba_count,
    spdk_nvme_cmd_cb cb_fn,
    void* cb_arg,
    uint32_t io_flags)
{
    return spdk_nvme_ns_cmd_write(
        ns, qpair, buffer, lba, lba_count, cb_fn, cb_arg, io_flags);
}

int
SpdkNvmeCaller::SpdkNvmeCtrlrCmdAbort(
    struct spdk_nvme_ctrlr* ctrlr,
    struct spdk_nvme_qpair* qpair,
    uint16_t cid,
    spdk_nvme_cmd_cb cb_fn,
    void* cb_arg)
{
    return spdk_nvme_ctrlr_cmd_abort(ctrlr, qpair, cid, cb_fn, cb_arg);
}

int
SpdkNvmeCaller::SpdkNvmeCtrlrCmdIoRaw(
    struct spdk_nvme_ctrlr* ctrlr,
    struct spdk_nvme_qpair* qpair,
    struct spdk_nvme_cmd* cmd,
    void* buf,
    uint32_t len,
    spdk_nvme_cmd_cb cb_fn,
    void* cb_arg)
{
    return spdk_nvme_ctrlr_cmd_io_raw(
        ctrlr, qpair, cmd, buf, len, cb_fn, cb_arg);
}

int
SpdkNvmeCaller::SpdkNvmeCtrlrCmdAdminRaw(
    struct spdk_nvme_ctrlr* ctrlr,
    struct spdk_nvme_cmd* cmd,
    void* buf,
    uint32_t len,
    spdk_nvme_cmd_cb cb_fn,
    void* cb_arg)
{
    return spdk_nvme_ctrlr_cmd_admin_raw(ctrlr, cmd, buf, len, cb_fn, cb_arg);
}

int
SpdkNvmeCaller::SpdkNvmeCtrlrCmdGetLogPage(
    struct spdk_nvme_ctrlr* ctrlr,
    uint8_t log_page,
    uint32_t nsid,
    void* payload,
    uint32_t payload_size,
    uint64_t offset,
    spdk_nvme_cmd_cb cb_fn,
    void* cb_arg)
{
    return spdk_nvme_ctrlr_cmd_get_log_page(
        ctrlr, log_page, nsid, payload, payload_size, offset, cb_fn, cb_arg);
}

bool
SpdkNvmeCaller::SpdkNvmeCtrlrIsFailed(struct spdk_nvme_ctrlr* ctrlr)
{
    return spdk_nvme_ctrlr_is_failed(ctrlr);
}

int32_t
SpdkNvmeCaller::SpdkNvmeCtrlrProcessAdminCompletions(
    struct spdk_nvme_ctrlr* ctrlr)
{
    return spdk_nvme_ctrlr_process_admin_completions(ctrlr);
}

int32_t
SpdkNvmeCaller::SpdkNvmeQpairProcessCompletions(
    struct spdk_nvme_qpair* qpair, uint32_t max_completions)
{
    return spdk_nvme_qpair_process_completions(qpair, max_completions);
}

uint64_t
SpdkNvmeCaller::SpdkNvmeNsGetSize(struct spdk_nvme_ns* ns)
{
    return spdk_nvme_ns_get_size(ns);
}

struct spdk_nvme_qpair*
SpdkNvmeCaller::SpdkNvmeCtrlrAllocIoQpair(
    struct spdk_nvme_ctrlr* ctrlr,
    const struct spdk_nvme_io_qpair_opts* user_opts,
    size_t opts_size)
{
    return spdk_nvme_ctrlr_alloc_io_qpair(ctrlr, user_opts, opts_size);
}

int
SpdkNvmeCaller::SpdkNvmeCtrlrFreeIoQpair(struct spdk_nvme_qpair* qpair)
{
    return spdk_nvme_ctrlr_free_io_qpair(qpair);
}

uint32_t
SpdkNvmeCaller::SpdkNvmeNsGetSectorSize(struct spdk_nvme_ns* ns)
{
    return spdk_nvme_ns_get_sector_size(ns);
}
const struct spdk_nvme_ctrlr_data*
SpdkNvmeCaller::SpdkNvmeCtrlrGetData(struct spdk_nvme_ctrlr* ctrlr)
{
    return spdk_nvme_ctrlr_get_data(ctrlr);
}

struct spdk_pci_device*
SpdkNvmeCaller::SpdkNvmeCtrlrGetPciDevice(struct spdk_nvme_ctrlr* ctrlr)
{
    return spdk_nvme_ctrlr_get_pci_device(ctrlr);
}

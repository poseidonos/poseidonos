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

#include <cstdint>

#include "spdk/nvme.h"

namespace pos
{
class SpdkNvmeCaller
{
public:
    SpdkNvmeCaller(void);
    virtual ~SpdkNvmeCaller(void);
    virtual uint32_t SpdkNvmeNsGetId(struct spdk_nvme_ns* ns);
    virtual struct spdk_nvme_ctrlr* SpdkNvmeNsGetCtrlr(
        struct spdk_nvme_ns* ns);
    virtual int SpdkNvmeNsCmdDatasetManagement(
        struct spdk_nvme_ns* ns,
        struct spdk_nvme_qpair* qpair,
        uint32_t type,
        const struct spdk_nvme_dsm_range* range,
        uint16_t num_ranges,
        spdk_nvme_cmd_cb cb_fn,
        void* cb_arg);
    virtual int SpdkNvmeNsCmdRead(
        struct spdk_nvme_ns* ns,
        struct spdk_nvme_qpair* qpair,
        void* buffer,
        uint64_t lba,
        uint32_t lba_count,
        spdk_nvme_cmd_cb cb_fn,
        void* cb_arg,
        uint32_t io_flags);
    virtual int SpdkNvmeNsCmdWrite(
        struct spdk_nvme_ns* ns,
        struct spdk_nvme_qpair* qpair,
        void* buffer,
        uint64_t lba,
        uint32_t lba_count,
        spdk_nvme_cmd_cb cb_fn,
        void* cb_arg,
        uint32_t io_flags);
    virtual int SpdkNvmeCtrlrCmdAbort(
        struct spdk_nvme_ctrlr* ctrlr,
        struct spdk_nvme_qpair* qpair,
        uint16_t cid,
        spdk_nvme_cmd_cb cb_fn,
        void* cb_arg);
    virtual int SpdkNvmeCtrlrCmdIoRaw(
        struct spdk_nvme_ctrlr* ctrlr,
        struct spdk_nvme_qpair* qpair,
        struct spdk_nvme_cmd* cmd,
        void* buf,
        uint32_t len,
        spdk_nvme_cmd_cb cb_fn,
        void* cb_arg);
    virtual int SpdkNvmeCtrlrCmdAdminRaw(
        struct spdk_nvme_ctrlr* ctrlr,
        struct spdk_nvme_cmd* cmd,
        void* buf,
        uint32_t len,
        spdk_nvme_cmd_cb cb_fn,
        void* cb_arg);
    virtual int SpdkNvmeCtrlrCmdGetLogPage(
        struct spdk_nvme_ctrlr* ctrlr,
        uint8_t log_page,
        uint32_t nsid,
        void* payload,
        uint32_t payload_size,
        uint64_t offset,
        spdk_nvme_cmd_cb cb_fn,
        void* cb_arg);
    virtual bool SpdkNvmeCtrlrIsFailed(struct spdk_nvme_ctrlr* ctrlr);
    virtual int32_t SpdkNvmeCtrlrProcessAdminCompletions(
        struct spdk_nvme_ctrlr* ctrlr);
    virtual int32_t SpdkNvmeQpairProcessCompletions(
        struct spdk_nvme_qpair* qpair, uint32_t max_completions);
    virtual uint64_t SpdkNvmeNsGetSize(struct spdk_nvme_ns* ns);
};

} // namespace pos

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

#include <atomic>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "spdk/nvme.h"
#include "spdk/nvme_intel.h"
#include "spdk/thread.h"
#include "src/device/device_monitor.h"
#include "src/lib/system_timeout_checker.h"
#include "src/include/smart_ptr_type.h"

using namespace std;

struct spdk_nvme_ctrl;
struct spdk_nvme_ctrlr_data;
struct spdk_nvme_qpair;
struct spdk_nvme_registers;

namespace pos
{
class UBlockDevice;

static const uint32_t MAX_NAME_LENGTH = 1024;
static const uint32_t MAX_NS_NAME_LENGTH = 44;
static const uint32_t MAX_TR_ADDR_LENGTH = SPDK_NVMF_TRADDR_MAX_LEN + 1;

using TimeoutHandlerFunction = function<void(struct spdk_nvme_ctrlr*, struct spdk_nvme_qpair*, uint16_t)>;

using MappingFunctionForIOTimeoutHandler = function<void(UblockSharedPtr ublock, struct spdk_nvme_qpair* ioQpair)>;
using UnmappingFunctionForIOTimeoutHandler = function<void(struct spdk_nvme_qpair* ioQpair)>;

struct CtrlrEntry
{
    struct spdk_nvme_ctrlr* ctrlr;
    struct spdk_nvme_intel_rw_latency_page* latencyPage;
    char name[MAX_NAME_LENGTH];
};

enum EntryType
{
    EntryType_NvmeNs,
    EntryType_AioFile,
};

struct NsEntry
{
    EntryType type;

    union {
        struct
        {
            struct spdk_nvme_ctrlr* ctrlr;
            struct spdk_nvme_ns* ns;
        } nvme;
    } u;

    uint32_t ioSizeBlocks;
    uint64_t sizeInIos;
    char name[MAX_NAME_LENGTH];
    char trAddr[MAX_TR_ADDR_LENGTH];
};

enum class RetryType
{
    RETRY_TYPE_NONE = 0,
    RETRY_TYPE_FRONTEND,
    RETRY_TYPE_BACKEND,
    RETRY_TYPE_COUNT,
};

class Nvme : public DeviceMonitor
{
public:
    typedef void (*SpdkAttachEvent)(struct spdk_nvme_ns* ns, int num_devs,
        const spdk_nvme_transport_id* trid);
    typedef void (*SpdkDetachEvent)(string sn);

    virtual std::list<NsEntry*>* InitController(void);

    void Start(void) override;
    void Stop(void) override;
    void Pause(void) override;
    void Resume(void) override;
    bool IsPaused(void) override;

    void
    SetCallback(SpdkAttachEvent attach, SpdkDetachEvent detach)
    {
        attachCb = attach;
        detachCb = detach;
    }

    explicit Nvme(string monitor_name)
    : DeviceMonitor(monitor_name)
    {
    }

    virtual ~Nvme(void);

    static void SpdkDetach(void* arg1);
    static void Cleanup(void* arg1);
    uint32_t GetRetryCount(RetryType retryType);
    static void RegisterTimeoutHandlerFunc(TimeoutHandlerFunction timeoutAbortFunc,
        TimeoutHandlerFunction resetFunc);
    static void ControllerTimeoutCallback(void* cb_arg, struct spdk_nvme_ctrlr* ctrlr,
        struct spdk_nvme_qpair* qpair, uint16_t cid);

private:
    static bool _ProbeCallback(void* cb_ctx,
        const spdk_nvme_transport_id* trid,
        spdk_nvme_ctrlr_opts* opts);
    static void _InitScanCallback(void* cb_ctx,
        const spdk_nvme_transport_id* trid,
        spdk_nvme_ctrlr* ctrlr,
        const spdk_nvme_ctrlr_opts* opts);
    static void _AttachCallback(void* cb_ctx,
        const spdk_nvme_transport_id* trid,
        spdk_nvme_ctrlr* ctrlr,
        const spdk_nvme_ctrlr_opts* opts);
    static void _RemoveCallback(void* cbCtx, struct spdk_nvme_ctrlr* ctrlr);

    static void _RegisterNamespace(struct spdk_nvme_ctrlr* ctrlr,
        struct spdk_nvme_ns* ns,
        const char* trAddr);
    static void _RegisterController(struct spdk_nvme_ctrlr* ctrlr,
        const char* trAddr);

    static TimeoutHandlerFunction timeoutAbortHandler;
    static TimeoutHandlerFunction resetHandler;

    static void _Initialize();

    void _Monitoring();
    static SpdkAttachEvent attachCb;
    static SpdkDetachEvent detachCb;

    static std::atomic<bool> paused;
    static std::atomic<bool> triggerSpdkDetach;
    static std::mutex nvmeMutex;

    static uint64_t ctrlTimeoutInUs;
    static const uint64_t DEFAULT_TIMEOUT_IN_US;
    static const uint32_t DEFAULT_RETRY;

    static const uint64_t MAX_TIMEOUT_IN_US;
    static const uint32_t MAX_RETRY;

    static uint32_t retryCount[static_cast<int>(RetryType::RETRY_TYPE_COUNT)];

    static int numNamespaces;
    static int nsid;
    static std::list<CtrlrEntry*> controllers;
    static std::list<NsEntry*> namespaces;
};

} // namespace pos

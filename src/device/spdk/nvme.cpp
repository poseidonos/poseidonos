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

#include "nvme.hpp"

#include <sys/stat.h>
#include <unistd.h>

#include <bitset>
#include <cassert>
#include <future>
#include <iostream>
#include <string>
#include <vector>

#include "rte_malloc.h"
#include "spdk/nvme.h"
#include "spdk/nvme_spec.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/general_io/affinity_manager.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/scheduler/event_argument.h"

using namespace std;

namespace ibofos
{
int Nvme::numNamespaces;
int Nvme::nsid = 0;
std::list<CtrlrEntry*> Nvme::controllers;
std::list<NsEntry*> Nvme::namespaces;

uint32_t ioSizeBytes = 4096;

Nvme::SpdkAttachEvent Nvme::attachCb = nullptr;
Nvme::SpdkDetachEvent Nvme::detachCb = nullptr;
std::atomic<bool> Nvme::paused;
std::atomic<bool> Nvme::triggerSpdkDetach;
std::mutex Nvme::nvmeMutex;

TimeoutHandlerFunction Nvme::timeoutAbortHandler;
TimeoutHandlerFunction Nvme::resetHandler;
struct spdk_thread* Nvme::monitoringThread;

uint64_t
    Nvme::ctrlTimeoutInUs;

const uint64_t Nvme::DEFAULT_TIMEOUT_IN_US = 80000000ULL;
const uint64_t Nvme::MAX_TIMEOUT_IN_US = 100000000ULL;

const uint32_t Nvme::DEFAULT_RETRY = 0;
const uint32_t Nvme::MAX_RETRY = 10;

EnableAbortFunction Nvme::enableAbortFunc;
EnableAbortFunction Nvme::disableAbortFunc;

uint32_t Nvme::retryCount[static_cast<int>(RetryType::RETRY_TYPE_COUNT)];

void
Nvme::SubsystemReset(struct spdk_nvme_ctrlr* ctrlr)
{
    volatile struct spdk_nvme_registers* reg =
        spdk_nvme_ctrlr_get_registers(ctrlr);
    reg->nssr = SPDK_NVME_NSSR_VALUE;
}

void
Nvme::RegisterTimeoutHandlerFunc(TimeoutHandlerFunction timeoutAbortFunc,
    TimeoutHandlerFunction resetFunc)
{
    timeoutAbortHandler = timeoutAbortFunc;
    resetHandler = resetFunc;
}

void
Nvme::RegisterEnableAbortFunction(EnableAbortFunction inputEnableAbortFunc, 
                                  EnableAbortFunction inputDisableAbortFunc)
{
    enableAbortFunc = inputEnableAbortFunc;
    disableAbortFunc = inputDisableAbortFunc;
}

void
Nvme::_Initialize()
{
    ConfigManager& configManager = *ConfigManagerSingleton::Instance();
    std::string module("user_nvme_driver");

    bool enabled = false;

    int ret = configManager.GetValue(module, "use_config", &enabled,
        CONFIG_TYPE_BOOL);

    // Default Configuration

    ctrlTimeoutInUs = DEFAULT_TIMEOUT_IN_US;
    retryCount[static_cast<uint32_t>(RetryType::RETRY_TYPE_BACKEND)] = DEFAULT_RETRY;
    retryCount[static_cast<uint32_t>(RetryType::RETRY_TYPE_FRONTEND)] = DEFAULT_RETRY;

    int eventIdSuccess = static_cast<int>(IBOF_EVENT_ID::SUCCESS);
    if (ret != eventIdSuccess || enabled == false)
    {
        return;
    }

    uint64_t ctrlTimeout = 0;
    ret = configManager.GetValue(module, "ssd_timeout_us", &ctrlTimeout,
        CONFIG_TYPE_UINT64);

    if (ret == eventIdSuccess)
    {
        if (ctrlTimeout > MAX_TIMEOUT_IN_US)
        {
            IBOF_TRACE_WARN(IBOF_EVENT_ID::UNVME_MAX_TIMEOUT_EXCEED,
                "SSD Timeout usec From Configuration File Excceds Max timeout : {}, Input : {}",
                MAX_TIMEOUT_IN_US, ctrlTimeout);
            ctrlTimeout = MAX_TIMEOUT_IN_US;
        }
        ctrlTimeoutInUs = ctrlTimeout;
    }

    uint32_t inputRetryCount = 0;
    ret = configManager.GetValue(module, "retry_count_backend_io", &inputRetryCount,
        CONFIG_TYPE_UINT32);

    if (ret == eventIdSuccess)
    {
        if (inputRetryCount > MAX_RETRY)
        {
            IBOF_TRACE_WARN(IBOF_EVENT_ID::UNVME_MAX_RETRY_EXCEED,
                "Backend's Retry Count from Configuration File Excceds Max Retry Count : {}, Input : {}",
                MAX_RETRY, inputRetryCount);
            inputRetryCount = MAX_RETRY;
        }
        retryCount[static_cast<uint32_t>(RetryType::RETRY_TYPE_BACKEND)] = inputRetryCount;
    }

    ret = configManager.GetValue(module, "retry_count_frontend_io", &inputRetryCount,
        CONFIG_TYPE_UINT32);

    if (ret == eventIdSuccess)
    {
        if (inputRetryCount > MAX_RETRY)
        {
            IBOF_TRACE_DEBUG(IBOF_EVENT_ID::UNVME_MAX_RETRY_EXCEED,
                "Frontend's Retry Count from Configuration File Excceds Max Retry Count : {}, Input : {}",
                MAX_RETRY, inputRetryCount);
            inputRetryCount = MAX_RETRY;
        }
        retryCount[static_cast<uint32_t>(RetryType::RETRY_TYPE_FRONTEND)] = inputRetryCount;
    }
}

uint32_t
Nvme::GetRetryCount(RetryType retryType)
{
    return retryCount[static_cast<int>(retryType)];
}

void
Nvme::Start()
{
    if (isRunning == false)
    {
        IBOF_TRACE_DEBUG(IBOF_EVENT_ID::UNVME_DAEMON_START, "spdk daemon started");
        isRunning = true;
        _Monitoring();
    }
}

void
Nvme::Stop()
{
    Resume();
    if (isRunning == true)
    {
        IBOF_TRACE_DEBUG(IBOF_EVENT_ID::UNVME_DAEMON_FINISH, "spdk daemon stopped");
        isRunning = false;
    }
}

spdk_thread*
Nvme::GetSpdkThread()
{
    return monitoringThread;
}

void
Nvme::_Monitoring()
{
    IBOF_TRACE_INFO(IBOF_EVENT_ID::UNVME_DAEMON_START, "spdk daemon monitoring started");
    AffinityManagerSingleton::Instance()->SetGeneralAffinitySelf();
    monitoringThread = spdk_thread_create_without_registered_fn("DeviceMonitor");

    while (isRunning)
    {
        if (paused)
        {
            continue;
        }

        if (!isRunning)
        {
            break;
        }

        if (spdk_nvme_probe(NULL, NULL, _ProbeCallback,
                _AttachCallback, _RemoveCallback) != 0)
        {
            IBOF_TRACE_ERROR(IBOF_EVENT_ID::UNVME_DAEMON_FINISH, "spdk daemon stopped unexpectedly");
            break;
        }

        usleep(10000);
    }

    IBOF_TRACE_DEBUG(IBOF_EVENT_ID::UNVME_DAEMON_FINISH, "spdk daemon monitoring stopped");
    while (spdk_thread_poll(monitoringThread, 0, 0) > 0)
    {
        usleep(1);
    }
    Cleanup(nullptr);
    spdk_set_thread(monitoringThread);
    spdk_thread_exit(monitoringThread);
    spdk_thread_destroy(monitoringThread);
}

bool
Nvme::_ProbeCallback(void* cbCtx,
    const spdk_nvme_transport_id* trid,
    spdk_nvme_ctrlr_opts* opts)
{
    IBOF_TRACE_INFO(IBOF_EVENT_ID::UNVME_PROBE_CALLBACK, "Probing {} ", trid->traddr);
    return true;
}

void
Nvme::_RegisterNamespace(
    struct spdk_nvme_ctrlr* ctrlr,
    struct spdk_nvme_ns* ns,
    const char* trAddr)
{
    NsEntry* entry;
    const struct spdk_nvme_ctrlr_data* ctrlrData;

    ctrlrData = spdk_nvme_ctrlr_get_data(ctrlr);
    IBOF_TRACE_INFO(IBOF_EVENT_ID::UNVME_REGISTER_NS, "Controller - MDTS: {}", ctrlrData->mdts);
    if (!spdk_nvme_ns_is_active(ns))
    {
        IBOF_TRACE_INFO(IBOF_EVENT_ID::UNVME_REGISTER_NS,
            "Controller {} ({}): Skipping inactive NS {}\n",
            ctrlrData->mn, ctrlrData->sn,
            spdk_nvme_ns_get_id(ns));
        return;
    }

    if (spdk_nvme_ns_get_size(ns) < ioSizeBytes ||
        spdk_nvme_ns_get_sector_size(ns) > ioSizeBytes)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::UNVME_REGISTER_NS,
            "WARNING: controller {} ({}) ns {} has invalid "
            "ns size {} / block size {} for I/O size {}\n",
            ctrlrData->mn, ctrlrData->sn, spdk_nvme_ns_get_id(ns),
            spdk_nvme_ns_get_size(ns), spdk_nvme_ns_get_sector_size(ns), ioSizeBytes);
        return;
    }

    entry = (NsEntry*)malloc(sizeof(NsEntry));
    if (entry == NULL)
    {
        IBOF_TRACE_ERROR(IBOF_EVENT_ID::UNVME_REGISTER_NS, "ns_entry malloc");
        exit(1);
    }

    entry->type = EntryType_NvmeNs;
    entry->u.nvme.ctrlr = ctrlr;
    entry->u.nvme.ns = ns;

    entry->sizeInIos = spdk_nvme_ns_get_size(ns) / ioSizeBytes;
    entry->ioSizeBlocks = ioSizeBytes / spdk_nvme_ns_get_sector_size(ns);

    snprintf(entry->name, 44, "%-20.20s (%-20.20s)", ctrlrData->mn, ctrlrData->sn);
    memcpy(entry->trAddr, trAddr, MAX_TR_ADDR_LENGTH);

    numNamespaces++;
    nsid++;
    namespaces.push_back(entry);
}

/**
 * Transport address of the NVMe-oF endpoint. For transports which use IP
 * addressing (e.g. RDMA), this should be an IP address. For PCIe, this
 * can either be a zero length string (the whole bus) or a PCI address
 * in the format DDDD:BB:DD.FF or DDDD.BB.DD.FF. For FC the string is
 * formatted as: nn-0xWWNN:pn-0xWWPN” where WWNN is the Node_Name of the
 * target NVMe_Port and WWPN is the N_Port_Name of the target NVMe_Port.
 */
void
Nvme::_RegisterController(struct spdk_nvme_ctrlr* ctrlr, const char* trAddr)
{
    std::lock_guard<std::mutex> guard(nvmeMutex);

    static bool calledConfigParser = false;
    if (calledConfigParser == false)
    {
        _Initialize();
        calledConfigParser = true;
    }
    CtrlrEntry* entry = (CtrlrEntry*)malloc(sizeof(CtrlrEntry));
    const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(ctrlr);

    if (entry == NULL)
    {
        IBOF_TRACE_ERROR(IBOF_EVENT_ID::UNVME_REGISTER_CTRL, "ctrlr_entry malloc");
        exit(1);
    }

    entry->latencyPage = (struct spdk_nvme_intel_rw_latency_page*)rte_zmalloc(
        "nvme latency", sizeof(struct spdk_nvme_intel_rw_latency_page),
        4096);
    if (entry->latencyPage == NULL)
    {
        IBOF_TRACE_ERROR(IBOF_EVENT_ID::UNVME_REGISTER_CTRL, "Allocation error (latency page)");
        exit(1);
    }

    snprintf(entry->name, sizeof(entry->name), "%-20.20s (%-20.20s)", ctrlrData->mn, ctrlrData->sn);

    entry->ctrlr = ctrlr;
    controllers.push_back(entry);

    spdk_nvme_ctrlr_register_timeout_callback(ctrlr, ctrlTimeoutInUs,
        &ControllerTimeoutCallback, nullptr);

    int numNs = spdk_nvme_ctrlr_get_num_ns(ctrlr);
    for (int i = 1; i <= numNs; i++)
    {
        _RegisterNamespace(ctrlr, spdk_nvme_ctrlr_get_ns(ctrlr, i), trAddr);
    }
}

void
Nvme::_InitScanCallback(void* cbCtx,
    const spdk_nvme_transport_id* trid,
    spdk_nvme_ctrlr* ctrlr,
    const spdk_nvme_ctrlr_opts* opts)
{
    IBOF_TRACE_INFO(IBOF_EVENT_ID::UNVME_INIT_SCAN_CALLBACK,
        "Attaching to {}", trid->traddr);
    _RegisterController(ctrlr, trid->traddr);
}

void
Nvme::_AttachCallback(void* cbCtx,
    const spdk_nvme_transport_id* trid,
    spdk_nvme_ctrlr* ctrlr,
    const spdk_nvme_ctrlr_opts* opts)
{
    IBOF_TRACE_WARN(IBOF_EVENT_ID::UNVME_ATTACH_CALLBACK,
        "spdk device attachment detected");

    _RegisterController(ctrlr, trid->traddr);
    if (attachCb != nullptr)
    {
        paused = true;
        disableAbortFunc();
        struct spdk_nvme_ns* ns = spdk_nvme_ctrlr_get_ns(ctrlr, 1);
        auto f = async(launch::async, attachCb, ns, nsid - 1, trid);
        while (paused)
        {
            usleep(1);
        }
        enableAbortFunc();
    }
    else
    {
        IBOF_TRACE_ERROR(IBOF_EVENT_ID::UNVME_ATTACH_CALLBACK,
            "set device event callback");
    }
}

bool
Nvme::IsPaused()
{
    return (paused);
}

void
Nvme::Pause()
{
    paused = true;
}

void
Nvme::Resume()
{
    paused = false;
}

void
Nvme::_RemoveCallback(void* cbCtx, struct spdk_nvme_ctrlr* ctrlr)
{
    IBOF_TRACE_WARN(IBOF_EVENT_ID::UNVME_DETACH_CALLBACK,
        "spdk device detachment detected");
    if (detachCb != nullptr)
    {
        const struct spdk_nvme_ctrlr_data* cdata;
        cdata = spdk_nvme_ctrlr_get_data(ctrlr);
        char sn[128];
        snprintf(sn, sizeof(cdata->sn) + 1, "%s", cdata->sn);
        struct spdk_nvme_ns* ns = spdk_nvme_ctrlr_get_ns(ctrlr, 1);

        paused = true;
        disableAbortFunc();
        auto f = async(launch::async, detachCb, string(sn));
        while (paused)
        {
            usleep(1);
        }
        enableAbortFunc();

        SpdkDetach(ns);
    }
    else
    {
        IBOF_TRACE_ERROR(IBOF_EVENT_ID::UNVME_DETACH_CALLBACK,
            "set device event callback");
    }
}

void
Nvme::SpdkDetach(void* arg1)
{
    struct spdk_nvme_ns* ns = static_cast<struct spdk_nvme_ns*>(arg1);
    if (ns == NULL)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::UNVME_SPDK_DETACH,
            "SpdkDetach - ns == NULL");
        return;
    }

    struct spdk_nvme_ctrlr* ctrlr = spdk_nvme_ns_get_ctrlr(ns);

    std::lock_guard<std::mutex> guard(nvmeMutex);
    for (std::list<NsEntry*>::iterator iter = namespaces.begin();
         iter != namespaces.end(); iter++)
    {
        if ((*iter)->u.nvme.ns == ns)
        {
            IBOF_TRACE_INFO(IBOF_EVENT_ID::UNVME_SPDK_DETACH, "SpdkDetach - free NsEntry");
            // NsEntry *next = nsEntry->next;

            free(*iter);
            namespaces.erase(iter);

            numNamespaces--;
            break;
        }
    }

    for (std::list<CtrlrEntry*>::iterator iter = controllers.begin();
         iter != controllers.end(); iter++)
    {
        if ((*iter)->ctrlr == ctrlr)
        {
            IBOF_TRACE_INFO(IBOF_EVENT_ID::UNVME_SPDK_DETACH, "SpdkDetach - free ctrlr");
            spdk_nvme_detach((*iter)->ctrlr);
            free(*iter);

            controllers.erase(iter);

            break;
        }
    }

    IBOF_TRACE_INFO(IBOF_EVENT_ID::UNVME_SPDK_DETACH, "SpdkDetach done");
}

void
Nvme::Cleanup(void* arg1)
{
    std::lock_guard<std::mutex> guard(nvmeMutex);
    IBOF_TRACE_INFO(IBOF_EVENT_ID::UNVME_CLEAN_UP, "Detaching SPDK controlllers!");

    for (std::list<NsEntry*>::iterator iter = namespaces.begin();
         iter != namespaces.end(); iter++)
    {
        free(*iter);
    }

    for (std::list<CtrlrEntry*>::iterator iter = controllers.begin();
         iter != controllers.end(); iter++)
    {
        spdk_nvme_detach((*iter)->ctrlr);
        free(*iter);
    }
}

std::list<NsEntry*>*
Nvme::InitController(void)
{
    /*
     * Start the SPDK NVMe enumeration process.  probe_cb will be called
     *  for each NVMe controller found, giving our application a choice on
     *  whether to attach to each controller.  attach_cb will then be
     *  called for each controller after the SPDK NVMe driver has completed
     *  initializing the controller we chose to attach.
     */

    int rc = spdk_nvme_probe(NULL, NULL, _ProbeCallback, _InitScanCallback, _RemoveCallback);
    if (rc != 0)
    {
        IBOF_TRACE_ERROR(IBOF_EVENT_ID::UNVME_INIT_CONTROLLER, "spdk_nvme_probe() failed");
        Nvme::Cleanup(nullptr);
        return nullptr;
    }

    IBOF_TRACE_WARN(IBOF_EVENT_ID::UNVME_INIT_CONTROLLER, "Initialization complete.");

    return &namespaces;
}

void
Nvme::ControllerTimeoutCallback(void* cbArg, struct spdk_nvme_ctrlr* ctrlr,
    struct spdk_nvme_qpair* qpair, uint16_t cid)
{
    const struct spdk_nvme_ctrlr_data* ctrlData =
        spdk_nvme_ctrlr_get_data(ctrlr);

    IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_COMPLETION_TIMEOUT;
    IBOF_TRACE_WARN(static_cast<int>(eventId),
        IbofEventId::GetString(eventId), ctrlData->sn);

    union spdk_nvme_csts_register csts = spdk_nvme_ctrlr_get_regs_csts(ctrlr);
    if (csts.bits.cfs)
    {
        eventId = IBOF_EVENT_ID::UNVME_CONTROLLER_FATAL_STATUS;
        IBOF_TRACE_WARN(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), ctrlData->sn);

        resetHandler(ctrlr, qpair, cid);
        return;
    }

    timeoutAbortHandler(ctrlr, qpair, cid);
}

Nvme::~Nvme(void)
{
}

} // namespace ibofos

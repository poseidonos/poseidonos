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

#include "command_timeout_handler.h"

#include <functional>
#include <string>

#include "spdk/nvme.h"
#include "src/array/service/array_service_layer.h"
#include "src/device/base/ublock_device.h"
#include "src/device/unvme/unvme_drv.h"
#include "src/include/pos_event_id.hpp"
#include "src/bio/ubio.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/callback.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/spdk_wrapper/abort_context.h"

using namespace std;
using namespace std::placeholders;

namespace pos
{
std::unordered_map<uint64_t, SystemTimeoutChecker*> CommandTimeoutHandler::mapAbort;
std::mutex CommandTimeoutHandler::mapAbortMutex;
DeviceIterFunc CommandTimeoutHandler::resetDevice;

const uint64_t
    CommandTimeoutHandler::ABORT_TIMEOUT_IN_NS = 8000000000ULL;

CommandTimeoutHandler::CommandTimeoutHandler(void)
{
    timeoutAbortHandler = bind(&CommandTimeoutHandler::_TimeoutActionAbortHandler, this, _1, _2, _3);
    resetHandler = bind(&CommandTimeoutHandler::_ResetHandler, this, _1, _2, _3);
    resetDevice = bind(&CommandTimeoutHandler::_ResetDevice, this, _1, _2);
    Nvme::RegisterTimeoutHandlerFunc(timeoutAbortHandler,
        resetHandler);
}

// We check, Ublock is alive or not before resetting devices.
// This case only is necessary when abort's callback calls reset function.

void
CommandTimeoutHandler::_ResetDevice(UblockSharedPtr dev, void* ctx)
{
    AbortContext* abortContext = static_cast<AbortContext*>(ctx);
    const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(abortContext->ctrlr);

    if (0 == memcmp(ctrlrData->sn, dev->GetSN().c_str(), dev->GetSN().size()))
    {
        CommandTimeoutHandlerSingleton::Instance()->_ResetHandler(abortContext->ctrlr,
            abortContext->qpair, abortContext->cid);
    }
}

void
CommandTimeoutHandler::__AbortSubmitHandler::DiskIO(UblockSharedPtr dev, void *ctx)
{
    const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(abortContext->ctrlr);
    string devSerial = dev->GetSN();

    if (abortContext->ctrlr != nullptr
        && 0 == memcmp(ctrlrData->sn, devSerial.c_str(), dev->GetSN().size()))
    {
        UbioSmartPtr bio(new Ubio(abortContext, 1, 0));
        bio->dir = UbioDir::Abort;
        CallbackSmartPtr callback(new AbortCompletionHandler(abortContext));
        bio->SetLba(0);
        bio->SetUblock(dev);
        bio->SetCallback(callback);
        IODispatcher& ioDispatcher = *IODispatcherSingleton::Instance();
        ioDispatcher.Submit(bio);
    }
}

CommandTimeoutHandler::__AbortSubmitHandler::__AbortSubmitHandler(AbortContext* inputAbortContext, DeviceManager* devMgr)
: abortContext(inputAbortContext),
  devMgr(devMgr)
{
    // initialization
}

bool
CommandTimeoutHandler::IsPendingAbortZero(void)
{
    return (mapAbort.size() == 0);
}

bool
CommandTimeoutHandler::__AbortSubmitHandler::Execute(void)
{
    driverFunc = bind(&CommandTimeoutHandler::__AbortSubmitHandler::DiskIO, this, _1, _2);
    devMgr->IterateDevicesAndDoFunc(driverFunc, nullptr);
    return true;
}

CommandTimeoutHandler::AbortCompletionHandler::AbortCompletionHandler(AbortContext* inputAbortContext)
: Callback(false, CallbackType_AbortCompletionHandler),
  abortContext(inputAbortContext)
{
}

bool
CommandTimeoutHandler::AbortCompletionHandler::_DoSpecificJob(void)
{
    POS_EVENT_ID eventId = EID(UNVME_ABORT_COMPLETION);
    const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(abortContext->ctrlr);

    if (_GetErrorCount() > 0)
    {
        POS_EVENT_ID eventId = EID(UNVME_ABORT_COMPLETION_FAILED);
        const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(abortContext->ctrlr);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Failed to complete command abort: SN: {}", ctrlrData->sn);
        CommandTimeoutHandler::_TryResetHandler(abortContext->ctrlr, abortContext->qpair, abortContext->cid);
    }
    else
    {
        POS_TRACE_INFO(static_cast<int>(eventId),
            "successfully complete command abort: SN: {}", ctrlrData->sn);
    }

    CommandTimeoutHandler::_Delete(abortContext->ctrlr, abortContext->cid);

    delete abortContext;
    return true;
}

uint64_t
CommandTimeoutHandler::_GetKey(struct spdk_nvme_ctrlr* ctrlr, uint16_t cid)
{
    return ((uint64_t)ctrlr) | ((uint64_t)cid << ADDR_MAX_BIT);
}

void
CommandTimeoutHandler::_Delete(struct spdk_nvme_ctrlr* ctrlr, uint16_t cid)
{
    std::lock_guard<std::mutex> guard(mapAbortMutex);
    uint64_t key = _GetKey(ctrlr, cid);
    if (mapAbort.find(key) != mapAbort.end())
    {
        delete mapAbort[key];
        mapAbort.erase(key);
    }
}

void
CommandTimeoutHandler::_TimeoutActionAbortHandler(
    struct spdk_nvme_ctrlr* ctrlr,
    struct spdk_nvme_qpair* qpair, uint16_t cid)
{
    const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(ctrlr);

    std::lock_guard<std::mutex> guard(mapAbortMutex);
    uint64_t key = _GetKey(ctrlr, cid);

    // qpair nullptr means, admin command.
    // cid is unique in ctrlr domain for only admin command.
    // so, null check is necessary
    // Pending abort command is allocated on the map Abort

    if (qpair == nullptr)
    {
        POS_EVENT_ID eventId = EID(UNVME_ABORT_TIMEOUT);
        const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(ctrlr);
        POS_TRACE_INFO(static_cast<int>(eventId),
            "Abort Also Timeout: SN: {}, QPair: {}, CID: {}",
            ctrlrData->sn, reinterpret_cast<uint64_t>(qpair), cid);

        _ResetHandler(ctrlr, qpair, cid);
        return;
    }

    SystemTimeoutChecker* timeoutChecker = new SystemTimeoutChecker;
    mapAbort[key] = timeoutChecker;
    timeoutChecker->SetTimeout(ABORT_TIMEOUT_IN_NS);

    POS_EVENT_ID eventId = EID(UNVME_SUBMITTING_CMD_ABORT);
    POS_TRACE_INFO(static_cast<int>(eventId),
        "Requesting command abort: SN: {}, QPair: {}, CID: {}",
        ctrlrData->sn, reinterpret_cast<uint64_t>(qpair), cid);

    AbortContext* abortContext = new AbortContext(ctrlr, qpair, cid);
    EventSmartPtr event(new __AbortSubmitHandler(abortContext));
    EventSchedulerSingleton::Instance()->EnqueueEvent(event);
}

void
CommandTimeoutHandler::_ResetHandler(
    struct spdk_nvme_ctrlr* ctrlr,
    struct spdk_nvme_qpair* qpair, uint16_t cid)
{
    const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(ctrlr);
    // Todo:: We just ctrl state as removed and failed,
    // We can fix this issue in spdk 20.07
    spdk_nvme_ctrlr_fail_and_remove(ctrlr);
    POS_EVENT_ID eventId = EID(UNVME_CONTROLLER_RESET);
    POS_TRACE_ERROR(static_cast<int>(eventId),
        "Controller Reset : SN : {}", ctrlrData->sn);
}

// In callback function is called, device can be already detached.
// so, we protect the case that poseidonos access nullptr or weird pointer value which points detached dev.
void
CommandTimeoutHandler::_TryResetHandler(
    struct spdk_nvme_ctrlr* ctrlr,
    struct spdk_nvme_qpair* qpair, uint16_t cid)
{
    AbortContext abortContext(ctrlr, qpair, cid);
    DeviceManagerSingleton::Instance()->IterateDevicesAndDoFunc(resetDevice, &abortContext);
}

} // namespace pos

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

#include "command_timeout_handler.h"

#include <functional>
#include <string>

#include "spdk/nvme.h"
#include "src/array/array.h"
#include "src/array/device/array_device.h"
#include "src/device/ublock_device.h"
#include "src/device/unvme/unvme_drv.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/general_io/ubio.h"
#include "src/logger/logger.h"
#include "src/scheduler/callback.h"
#include "src/scheduler/event.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/io_dispatcher.h"

using namespace std;
using namespace std::placeholders;

namespace ibofos
{
std::unordered_map<uint64_t, SystemTimeoutChecker*> CommandTimeoutHandler::mapAbort;
std::mutex CommandTimeoutHandler::mapAbortMutex;
DeviceIterFunc CommandTimeoutHandler::resetDevice;

const uint64_t
    CommandTimeoutHandler::ABORT_TIMEOUT_IN_NS = 8000000000ULL;

CommandTimeoutHandler::CommandTimeoutHandler()
{
    abortDisabledCount = ABORT_DISABLED_INIT;
    timeoutAbortHandler = bind(&CommandTimeoutHandler::_TimeoutActionAbortHandler, this, _1, _2, _3);
    resetHandler = bind(&CommandTimeoutHandler::_ResetHandler, this, _1, _2, _3);
    resetDevice = bind(&CommandTimeoutHandler::_ResetDevice, this, _1, _2);
    enableAbortFunc = bind(&CommandTimeoutHandler::EnableAbort, this);
    disableAbortFunc = bind(&CommandTimeoutHandler::DisableAbort, this);
    Nvme::RegisterTimeoutHandlerFunc(timeoutAbortHandler,
        resetHandler);
    Nvme::RegisterEnableAbortFunction(enableAbortFunc, disableAbortFunc);
}

// We check, Ublock is alive or not before resetting devices.
// This case only is necessary when abort's callback calls reset function.

void
CommandTimeoutHandler::_ResetDevice(UBlockDevice* dev, void* ctx)
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
CommandTimeoutHandler::AbortSubmitHandler::DiskIO(UBlockDevice* dev, void* ctx)
{
    AbortContext* abortContext = static_cast<AbortContext*>(ctx);
    const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(abortContext->ctrlr);

    if (0 == memcmp(ctrlrData->sn, dev->GetSN().c_str(), dev->GetSN().size()))
    {
        UbioSmartPtr bio(new Ubio(abortContext, 1));

        bio->dir = UbioDir::Abort;
        ArrayDevice* arrayDev = new ArrayDevice(dev, ArrayDeviceState::NORMAL);
        CallbackSmartPtr callback(new AbortCompletionHandler(abortContext));
        PhysicalBlkAddr pba = {.dev = arrayDev, .lba = 0};
        bio->SetPba(pba);
        bio->SetCallback(callback);
        IODispatcher& ioDispatcher = *EventArgument::GetIODispatcher();
        ioDispatcher.Submit(bio);
    }
}

CommandTimeoutHandler::AbortSubmitHandler::AbortSubmitHandler(AbortContext* inputAbortContext)
: abortContext(inputAbortContext)
{
}
bool
CommandTimeoutHandler::IsPendingAbortZero(void)
{
    return (mapAbort.size() == 0);
}

bool
CommandTimeoutHandler::AbortSubmitHandler::Execute(void)
{
    driverFunc = bind(&CommandTimeoutHandler::AbortSubmitHandler::DiskIO, this, _1, _2);
    DeviceManagerSingleton::Instance()->IterateDevicesAndDoFunc(driverFunc, abortContext);
    return true;
}

CommandTimeoutHandler::AbortCompletionHandler::AbortCompletionHandler(AbortContext* inputAbortContext)
: Callback(false),
  abortContext(inputAbortContext)
{
}

bool
CommandTimeoutHandler::AbortCompletionHandler::_DoSpecificJob(void)
{
    IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_ABORT_COMPLETION;
    const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(abortContext->ctrlr);

    if (_GetErrorCount() > 0)
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_ABORT_COMPLETION_FAILED;
        const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(abortContext->ctrlr);
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), ctrlrData->sn);
        CommandTimeoutHandler::_TryResetHandler(abortContext->ctrlr, abortContext->qpair, abortContext->cid);
    }
    else
    {
        IBOF_TRACE_INFO(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), ctrlrData->sn);
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
CommandTimeoutHandler::_EnableAbort(bool flag)
{
    if (flag == false)
    {
        abortDisabledCount++;
    }
    else
    {
        abortDisabledCount--;
        if (abortDisabledCount < 0)
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_ABORT_DISABLE_COUNT_ERR;

            IBOF_TRACE_WARN(static_cast<int>(eventId),
                IbofEventId::GetString(eventId),
                abortDisabledCount);
        }
    }
}

void
CommandTimeoutHandler::EnableAbort()
{
    _EnableAbort(true);
}

void
CommandTimeoutHandler::DisableAbort()
{
    _EnableAbort(false);
}

void
CommandTimeoutHandler::_TimeoutActionAbortHandler(
    struct spdk_nvme_ctrlr* ctrlr,
    struct spdk_nvme_qpair* qpair, uint16_t cid)
{
    const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(ctrlr);

    std::lock_guard<std::mutex> guard(mapAbortMutex);
    uint64_t key = _GetKey(ctrlr, cid);

    if (abortDisabledCount > 0)
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_ABORT_DISABLE_AND_RESET;
        const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(ctrlr);
        IBOF_TRACE_INFO(static_cast<int>(eventId),
            IbofEventId::GetString(eventId),
            ctrlrData->sn, reinterpret_cast<uint64_t>(qpair), cid);
        _ResetHandler(ctrlr, qpair, cid);
        return;
    }
    // qpair nullptr means, admin command.
    // cid is unique in ctrlr domain for only admin command.
    // so, null check is necessary
    // Pending abort command is allocated on the map Abort

    if (qpair == nullptr && mapAbort.find(key) != mapAbort.end())
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_ABORT_TIMEOUT;
        const struct spdk_nvme_ctrlr_data* ctrlrData = spdk_nvme_ctrlr_get_data(ctrlr);
        IBOF_TRACE_INFO(static_cast<int>(eventId),
            IbofEventId::GetString(eventId),
            ctrlrData->sn, reinterpret_cast<uint64_t>(qpair), cid);

        _ResetHandler(ctrlr, qpair, cid);
        return;
    }

    SystemTimeoutChecker* timeoutChecker = new SystemTimeoutChecker;
    mapAbort[key] = timeoutChecker;
    timeoutChecker->SetTimeout(ABORT_TIMEOUT_IN_NS);

    IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_SUBMITTING_CMD_ABORT;
    IBOF_TRACE_INFO(static_cast<int>(eventId),
        IbofEventId::GetString(eventId),
        ctrlrData->sn, reinterpret_cast<uint64_t>(qpair), cid);

    AbortContext* abortContext = new AbortContext(ctrlr, qpair, cid);
    EventSmartPtr event(new AbortSubmitHandler(abortContext));
    EventArgument::GetEventScheduler()->EnqueueEvent(event);
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
    IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_CONTROLLER_RESET;
    IBOF_TRACE_ERROR(static_cast<int>(eventId),
        IbofEventId::GetString(eventId), ctrlrData->sn);
}

// In callback function is called, device can be already detached.
// so, we protect the case that ibofos access nullptr or weird pointer value which points detached dev.
void
CommandTimeoutHandler::_TryResetHandler(
    struct spdk_nvme_ctrlr* ctrlr,
    struct spdk_nvme_qpair* qpair, uint16_t cid)
{
    AbortContext abortContext(ctrlr, qpair, cid);
    DeviceManagerSingleton::Instance()->IterateDevicesAndDoFunc(resetDevice, &abortContext);
}

} // namespace ibofos

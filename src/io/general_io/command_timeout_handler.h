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
#include <mutex>
#include <unordered_map>

#include "src/device/device_manager.h"
#include "src/spdk_wrapper/nvme.hpp"
#include "src/device/unvme/unvme_drv.h"
#include "src/lib/system_timeout_checker.h"
#include "src/event_scheduler/callback.h"

namespace pos
{
class UBlockDevice;
class AbortContext;

class CommandTimeoutHandler
{
public:
    // Find device lock and increase pending io of deviceContext.
    // Without lock, that device can be removed at that time.
    // device->ioQpair is generated, ioQpair to device is generated.
    CommandTimeoutHandler(void);

    // test usage. pending abort zero is automatically checked by device context.
    bool IsPendingAbortZero(void);

    // We abort handling by "event", because the entry point of handler is "ioworker"
    // If abort and detach process are simulatneously happen, io worker requires "array state lock" and,
    // Event also requires "array state lock" and submit i/o to io worker and waiting with "array state lock"
    // That condition is prominently "dead lock" condition. To avoid dead lock, we submit abort command as "event"

    class __AbortSubmitHandler : public Event
    {
    public:
        __AbortSubmitHandler(AbortContext* inputAbortContext, DeviceManager* devMgr = DeviceManagerSingleton::Instance());
        void DiskIO(UblockSharedPtr dev, void* ctx);
        bool Execute(void) override;

    private:
        AbortContext* abortContext;
        DeviceManager* devMgr;
        DeviceIterFunc driverFunc;
    };

private:
    class AbortCompletionHandler : public Callback
    {
    public:
        explicit AbortCompletionHandler(AbortContext* inputAbortContext);

    private:
        bool _DoSpecificJob(void) override;
        AbortContext* abortContext;
    };

    void _TimeoutActionAbortHandler(
        struct spdk_nvme_ctrlr* ctrlr,
        struct spdk_nvme_qpair* qpair, uint16_t cid);

    void _ResetHandler(
        struct spdk_nvme_ctrlr* ctrlr,
        struct spdk_nvme_qpair* qpair, uint16_t cid);

    void _ResetDevice(UblockSharedPtr dev, void* ctx);

    static void _TryResetHandler(
        struct spdk_nvme_ctrlr* ctrlr,
        struct spdk_nvme_qpair* qpair, uint16_t cid);

    static uint64_t _GetKey(struct spdk_nvme_ctrlr* ctrlr, uint16_t cid);

    static void _Delete(struct spdk_nvme_ctrlr* ctrlr, uint16_t cid);


    static std::unordered_map<uint64_t, SystemTimeoutChecker*> mapAbort;


    TimeoutHandlerFunction timeoutAbortHandler;
    TimeoutHandlerFunction resetHandler;
    static DeviceIterFunc resetDevice;
    static const uint64_t ABORT_TIMEOUT_IN_NS;

    static std::mutex mapAbortMutex;

    // x86 uses 48bit virtual address space.
    static const uint32_t ADDR_MAX_BIT = 48;
};

using CommandTimeoutHandlerSingleton = Singleton<CommandTimeoutHandler>;

} // namespace pos

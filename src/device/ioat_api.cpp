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

#include "src/device/ioat_api.h"

#include <stdint.h>

#include "spdk/copy_engine.h"
#include "spdk/ioat.h"
#include "spdk/log.h"
#include "spdk/thread.h"
#include "src/device/event_framework_api.h"
#include "src/include/branch_prediction.h"
#include "src/include/ibof_event_id.hpp"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"

namespace ibofos
{
struct ioat_io_channel
{
    struct spdk_ioat_chan* ioat_ch;
    struct ioat_device* ioat_dev;
    struct spdk_poller* poller;
};

std::atomic<bool> IoatApi::initialized;
std::atomic<bool> IoatApi::finalized;
std::atomic<bool> IoatApi::enabled;
thread_local spdk_ioat_chan* IoatApi::ioatChannel;
thread_local spdk_io_channel* IoatApi::spdkChannel;

void
IoatApi::Initialize(void)
{
    initialized = false;

    _SetIoat();
    if (IsIoatEnable() == false)
    {
        IBOF_TRACE_INFO((int)IBOF_EVENT_ID::IOATAPI_DISABLED,
            "Ioat Copy Engine Offload Disabled");
        return;
    }

    IBOF_TRACE_INFO((int)IBOF_EVENT_ID::IOATAPI_DISABLED,
        "Ioat Copy Engine Offload Enabled");
    _HandleInitialize(nullptr, nullptr);

    while (initialized == false)
    {
        usleep(1);
    }
}

void
IoatApi::_SetChannel(void)
{
    spdkChannel = ioat_get_io_channel();
    if (spdkChannel == nullptr)
    {
        return;
    }
    ioat_io_channel* ioatChan =
        static_cast<ioat_io_channel*>(spdk_io_channel_get_ctx(spdkChannel));

    ioatChannel = ioatChan->ioat_ch;
}

void
IoatApi::_HandleInitialize(void* arg1, void* arg2)
{
    if (_IsIoatValidInSystem())
    {
        if (false == _IsChannelValid())
        {
            _SetChannel();
        }
    }

    if (EventFrameworkApi::IsLastReactorNow())
    {
        initialized = true;
    }
    else
    {
        uint32_t nextCore = EventFrameworkApi::GetNextReactor();
        bool success = EventFrameworkApi::SendSpdkEvent(nextCore,
            _HandleInitialize, nullptr, nullptr);
        if (unlikely(false == success))
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::IOATAPI_FAIL_TO_INITIALIZE;
            IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId));
        }
    }
}

bool
IoatApi::_IsChannelValid(void)
{
    return ioatChannel != nullptr;
}

bool
IoatApi::IsIoatEnable(void)
{
    return enabled;
}

void
IoatApi::_SetIoat(void)
{
    bool readVal = true;
    int ret = ConfigManagerSingleton::Instance()->GetValue("ioat", "enable",
        &readVal, CONFIG_TYPE_BOOL);
    if (ret == static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        enabled = readVal;
    }
    else
    {
        enabled = true;
    }

    if (enabled == false)
    {
        return;
    }
    int rc = spdk_ioat_init();
    if (rc != 0)
    {
        enabled = false;
    }
}

void
IoatApi::_CopyFallback(IoatArgument& ioatArg)
{
    memcpy(ioatArg.dst, ioatArg.src, ioatArg.bytes);
    ioatArg.cbFunction(ioatArg.cbArgument);
    if (ioatArg.state == NEED_FREE)
    {
        delete &ioatArg;
    }
}

void
IoatApi::SubmitCopy(void* dst, void* src, uint64_t bytes, IoatCb cbFunction,
    void* cbArgument)
{
    if (_IsChannelValid())
    {
        IoatArgument ioatArgument(dst, src, bytes, cbFunction, cbArgument,
            NOTING_TO_DO);

        _HandleCopy(&ioatArgument, nullptr);
    }
    else
    {
        IoatArgument* ioatArgument = new IoatArgument(dst, src, bytes,
            cbFunction, cbArgument, NEED_FREE);
        if (_IsIoatValidInSystem())
        {
            uint32_t targetCore = EventFrameworkApi::GetTargetReactor();
            bool success = EventFrameworkApi::SendSpdkEvent(targetCore,
                _HandleCopy, ioatArgument, nullptr);
            if (unlikely(false == success))
            {
                IBOF_EVENT_ID eventId =
                    IBOF_EVENT_ID::IOATAPI_FAIL_TO_SUBMIT_COPY;
                IBOF_TRACE_ERROR(static_cast<int>(eventId),
                    IbofEventId::GetString(eventId));
                _CopyFallback(*ioatArgument);
            }
        }
        else
        {
            _CopyFallback(*ioatArgument);
        }
    }
}

bool
IoatApi::_IsIoatValidInSystem(void)
{
    return spdk_check_ioat_initialized();
}

void
IoatApi::_HandleCopy(void* arg1, void* arg2)
{
    IoatArgument& ioatArgument = *static_cast<IoatArgument*>(arg1);
    if (_IsChannelValid())
    {
        spdk_ioat_submit_copy(ioatChannel,
            ioatArgument.cbArgument,
            ioatArgument.cbFunction,
            ioatArgument.dst,
            ioatArgument.src,
            ioatArgument.bytes);

        if (ioatArgument.state == NEED_FREE)
        {
            delete &ioatArgument;
        }
    }
    else
    {
        uint32_t targetCore = EventFrameworkApi::GetTargetReactor();
        bool success = EventFrameworkApi::SendSpdkEvent(targetCore,
            _HandleCopy, &ioatArgument, nullptr);
        if (unlikely(false == success))
        {
            IBOF_EVENT_ID eventId =
                IBOF_EVENT_ID::IOATAPI_FAIL_TO_SUBMIT_COPY;
            IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId));
            _CopyFallback(ioatArgument);
        }
    }
}

void
IoatApi::_PutChannel(void)
{
    spdk_put_io_channel(spdkChannel);
    spdkChannel = nullptr;
    ioatChannel = nullptr;
    SPDK_NOTICELOG("Ioat Copy Engine Offload Disabled\n");
}

void
IoatApi::_HandleFinalize(void* arg1, void* arg2)
{
    if (_IsChannelValid())
    {
        _PutChannel();
    }

    if (EventFrameworkApi::IsLastReactorNow())
    {
        finalized = true;
    }
    else
    {
        uint32_t nextCore = EventFrameworkApi::GetNextReactor();
        bool success = EventFrameworkApi::SendSpdkEvent(nextCore,
            _HandleFinalize, nullptr, nullptr);
        if (unlikely(false == success))
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::IOATAPI_FAIL_TO_FINALIZE;
            IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId));
        }
    }
}

void
IoatApi::Finalize(void)
{
    finalized = false;
    uint32_t firstReactor = EventFrameworkApi::GetFirstReactor();
    bool success = EventFrameworkApi::SendSpdkEvent(firstReactor,
        _HandleFinalize, nullptr, nullptr);

    if (unlikely(false == success))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::IOATAPI_FAIL_TO_FINALIZE;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
        return;
    }

    while (finalized == false)
    {
        usleep(1);
    }
}
} // namespace ibofos

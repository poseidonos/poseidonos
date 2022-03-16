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

#include "src/spdk_wrapper/accel_engine_api.h"

#include <stdint.h>

#include "spdk_internal/accel_engine.h"

extern "C"
{
#include "accel/ioat/pos_accel_engine_ioat.h"
}

#include "spdk/ioat.h"
#include "spdk/log.h"
#include "spdk/thread.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/include/array_config.h"

namespace pos
{
struct ioat_io_channel
{
    struct spdk_ioat_chan* ioat_ch;
    struct ioat_device* ioat_dev;
    struct spdk_poller* poller;
};

std::atomic<bool> AccelEngineApi::initialized;
std::atomic<bool> AccelEngineApi::finalized;
std::atomic<bool> AccelEngineApi::enabled;
thread_local spdk_io_channel* AccelEngineApi::spdkChannel;
uint32_t AccelEngineApi::ioatReactorArray[RTE_MAX_LCORE] = {
    0,
};
uint32_t AccelEngineApi::reactorArray[RTE_MAX_LCORE] = {
    0,
};
thread_local uint32_t AccelEngineApi::requestCount;
uint32_t AccelEngineApi::detected_numa_count = 0;

std::atomic<uint32_t> AccelEngineApi::ioatReactorCount;
std::atomic<uint32_t> AccelEngineApi::ioatReactorCountPerNode[RTE_MAX_NUMA_NODES];
std::atomic<uint32_t> AccelEngineApi::reactorCount;
void
AccelEngineApi::Initialize(void)
{
    initialized = false;

    _SetIoat();
    if (IsIoatEnable() == false)
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::IOATAPI_DISABLED,
            "Ioat Copy Engine Offload Disabled");
    }
    else
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::IOATAPI_DISABLED,
            "Ioat Copy Engine Offload Enabled");
    }
    uint32_t nextCore = EventFrameworkApiSingleton::Instance()->GetFirstReactor();
    bool success = EventFrameworkApiSingleton::Instance()->SendSpdkEvent(nextCore,
        _HandleInitialize, nullptr);
    if (success == false)
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::IOATAPI_DISABLED,
            "Ioat Copy Engine Offload Disabled Due To Lack Of Spdk Events");
        return;
    }
}

void
AccelEngineApi::_SetChannel(void)
{
    bool ioatChannelExist = false;
    struct spdk_io_channel* ioatChannel = spdk_accel_ioat_get_io_channel();
    if (ioatChannel != nullptr)
    {
        spdk_put_io_channel(ioatChannel);
        ioatChannelExist = true;
    }

    spdkChannel = spdk_accel_engine_get_io_channel();
    if (spdkChannel == nullptr)
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::IOATAPI_FAIL_TO_INITIALIZE;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Spdk cannot obtain any io channel from accel engine.");
        assert(0);
    }

    uint32_t currentReactor = EventFrameworkApiSingleton::Instance()->GetCurrentReactor();
    if (ioatChannelExist)
    {
        ioatReactorArray[ioatReactorCount] = currentReactor;
        ioatReactorCount++;
        uint32_t ioatCount = get_ioat_count_per_numa(detected_numa_count);
        if (ioatReactorCountPerNode[detected_numa_count] >= ioatCount)
        {
            detected_numa_count++;
            if (get_ioat_count_per_numa(detected_numa_count) == 0)
            {
                POS_EVENT_ID eventId =
                    POS_EVENT_ID::IOATAPI_FAIL_TO_INITIALIZE;
                POS_TRACE_ERROR(static_cast<int>(eventId),
                    "Spdk's ioat count from probe() and get_io_channel() is mismatched!");
            }
        }
        ioatReactorCountPerNode[detected_numa_count]++;
    }
    reactorArray[reactorCount] = currentReactor;
    reactorCount++;
}

int
AccelEngineApi::GetIoatReactorByIndex(uint32_t index)
{
    if (index >= ioatReactorCount)
    {
        return INVALID_REACTOR;
    }
    return ioatReactorArray[index];
}

int
AccelEngineApi::GetReactorByIndex(uint32_t index)
{
    if (index >= reactorCount)
    {
        return INVALID_REACTOR;
    }
    return reactorArray[index];
}

uint32_t
AccelEngineApi::GetIoatReactorCount(void)
{
    return ioatReactorCount;
}

uint32_t
AccelEngineApi::GetReactorCount(void)
{
    return reactorCount;
}

uint32_t
AccelEngineApi::GetIoatReactorCountPerNode(uint32_t node)
{
    return ioatReactorCountPerNode[node];
}

void
AccelEngineApi::_HandleInitialize(void* arg1)
{
    _SetChannel();

    if (EventFrameworkApiSingleton::Instance()->IsLastReactorNow())
    {
        initialized = true;
    }
    else
    {
        uint32_t nextCore = EventFrameworkApiSingleton::Instance()->GetNextReactor();
        bool success = EventFrameworkApiSingleton::Instance()->SendSpdkEvent(nextCore,
            _HandleInitialize, nullptr);
        if (unlikely(false == success))
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::IOATAPI_FAIL_TO_INITIALIZE;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                "Fail to initialize IOAT");
        }
    }
}

bool
AccelEngineApi::_IsChannelValid(void)
{
    return spdkChannel != nullptr;
}

bool
AccelEngineApi::IsIoatEnable(void)
{
    return enabled;
}

void
AccelEngineApi::_SetIoat(void)
{
    bool readVal = true;
    int ret = ConfigManagerSingleton::Instance()->GetValue("ioat", "enable",
        &readVal, CONFIG_TYPE_BOOL);
    if (ret == EID(SUCCESS))
    {
        enabled = readVal;
    }
    else
    {
        enabled = true;
    }

    if (enabled == false)
    {
        SPDK_NOTICELOG("spdk ioat is disabled by user\n");
        return;
    }

    int rc = spdk_ioat_init();
    if (rc != 0)
    {
        enabled = false;
        SPDK_NOTICELOG("spdk ioat cannot be enabled. fall back to software copy\n");
    }
}

static void
CopyDone(void* cbArgument, int status)
{
    IoatArgument* ioatArg = static_cast<IoatArgument*>(cbArgument);
    if (status != 0)
    {
        memcpy(ioatArg->dst, ioatArg->src, ioatArg->bytes);
    }
    ioatArg->cbFunction(ioatArg->cbArgument);
    delete ioatArg;
}

void
AccelEngineApi::SubmitCopy(void* dst, void* src, uint64_t bytes, IoatCb cbFunction,
    void* cbArgument)
{
    if (likely(_IsChannelValid()) && !(bytes % ArrayConfig::SECTOR_SIZE_BYTE))
    {
        IoatArgument* ioatArg = new IoatArgument(dst, src, bytes, cbFunction, cbArgument);
        spdk_accel_submit_copy(spdkChannel, dst, src, bytes, CopyDone, ioatArg);
    }
    else
    {
        memcpy(dst, src, bytes);
        cbFunction(cbArgument);
    }
}

void
AccelEngineApi::_PutChannel(void)
{
    spdk_put_io_channel(spdkChannel);
    spdkChannel = nullptr;
    SPDK_NOTICELOG("Ioat Copy Engine Offload Disabled\n");
}

void
AccelEngineApi::_HandleFinalize(void* arg1)
{
    if (_IsChannelValid())
    {
        _PutChannel();
    }

    if (EventFrameworkApiSingleton::Instance()->IsLastReactorNow())
    {
        finalized = true;
    }
    else
    {
        uint32_t nextCore = EventFrameworkApiSingleton::Instance()->GetNextReactor();
        bool success = EventFrameworkApiSingleton::Instance()->SendSpdkEvent(nextCore,
            _HandleFinalize, nullptr);
        if (unlikely(false == success))
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::IOATAPI_FAIL_TO_FINALIZE;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                "Fail to finalize IOAT");
        }
    }
}

void
AccelEngineApi::Finalize(EventFrameworkApi* eventFrameworkApi)
{
    if (nullptr == eventFrameworkApi)
    {
        eventFrameworkApi = EventFrameworkApiSingleton::Instance();
    }
    finalized = false;
    uint32_t firstReactor = eventFrameworkApi->GetFirstReactor();
    bool success = eventFrameworkApi->SendSpdkEvent(firstReactor,
        _HandleFinalize, nullptr);

    if (unlikely(false == success))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::IOATAPI_FAIL_TO_FINALIZE;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Fail to finalize IOAT");
        return;
    }

    while (finalized == false)
    {
        usleep(1);
    }
}
} // namespace pos

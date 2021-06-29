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

#include "nvmf_target_spdk.hpp"

#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "spdk/nvme.h"
#include "spdk/nvmf.h"
#include "spdk/stdinc.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/spdk_wrapper/spdk.hpp"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/network/nvmf_target.hpp"
#include "src/network/nvmf_volume_pos.hpp"

extern struct spdk_nvmf_tgt* g_spdk_nvmf_tgt;

namespace pos
{
struct EventContext*
AllocEventContext(PosNvmfEventDoneCallback_t callback, void* userArg)
{
    struct EventContext* e = (struct EventContext*)malloc(sizeof(struct EventContext));
    if (!e)
    {
        return NULL;
    }
    memset((char*)e, 0, sizeof(struct EventContext));
    e->userCallback = callback;
    e->userArg = userArg;
    return e;
}

void
FreeEventContext(struct EventContext* e)
{
    if (e)
    {
        if (e->eventArg1)
            free(e->eventArg1);
        if (e->eventArg2)
            free(e->eventArg2);
        free(e);
    }
}

static void
GenericCallback(const char* caller, void* arg, int status)
{
    if (status == NvmfCallbackStatus::SUCCESS)
    {
        SPDK_NOTICELOG("Success: %s\n", caller);
    }
    else
    {
        SPDK_ERRLOG("Failure: %s status=%d\n", caller, status);
    }

    struct EventContext* ctx = (struct EventContext*)arg;
    if (ctx)
    {
        if (ctx->userCallback)
        {
            ctx->userCallback(ctx->userArg, status);
        }
        FreeEventContext(ctx);
    }
}

static void
CreatePosBdevDone(void* arg, int status)
{
    GenericCallback(__FUNCTION__, arg, status);
}
static void
DeletePosBdevDone(void* arg, int status)
{
    GenericCallback(__FUNCTION__, arg, status);
}

static void
CreateTransportDone(void* arg, int status)
{
    GenericCallback(__FUNCTION__, arg, status);
}

static void
SubsystemStartDone(struct spdk_nvmf_subsystem* subsystem, void* arg, int status)
{
    GenericCallback(__FUNCTION__, arg, status);
}
static void
SubsystemStopDone(struct spdk_nvmf_subsystem* subsystem, void* arg, int status)
{
    spdk_nvmf_subsystem_destroy(subsystem);
    GenericCallback(__FUNCTION__, arg, status);
}

static void
SubsystemResumeDone(struct spdk_nvmf_subsystem* subsystem, void* arg, int status)
{
    GenericCallback(__FUNCTION__, arg, status);
}

void
ActivateSubsystem(void* arg1)
{
    int ret;
    struct spdk_nvmf_subsystem* subsystem = (struct spdk_nvmf_subsystem*)arg1;
    ret = spdk_nvmf_subsystem_set_pause_state_directly(subsystem);
    if (ret != 0)
    {
        SPDK_NOTICELOG("Failed to pause subsystem(%s) during activating subsystem : Retrying\n",
            spdk_nvmf_subsystem_get_nqn(subsystem));
        EventFrameworkApi::SendSpdkEvent(EventFrameworkApi::GetFirstReactor(),
            ActivateSubsystem, subsystem);
    }
    ret = spdk_nvmf_subsystem_resume(subsystem, nullptr, nullptr);
    if (ret != 0)
    {
        SPDK_ERRLOG("Failed to resume subsystem(%s) during activating subsystem \n",
            spdk_nvmf_subsystem_get_nqn(subsystem));
    }
}

static void
AttachNamespaceResumeDone(struct spdk_nvmf_subsystem* subsystem, void* arg, int status)
{
    if (status != NvmfCallbackStatus::SUCCESS)
    {
        SPDK_ERRLOG("Failed to resume subsystem(%s)\n",
            spdk_nvmf_subsystem_get_nqn(subsystem));
    }

    struct EventContext* ctx = (struct EventContext*)arg;
    uint32_t nsid = 0;
    sscanf((char*)ctx->eventArg2, "%u", &nsid);
    status = nsid;

    if (nsid > 0)
    {
        status = NvmfCallbackStatus::SUCCESS;
    }
    else
    {
        status = NvmfCallbackStatus::FAILED;
    }

    GenericCallback(__FUNCTION__, arg, status);
}

static void
AttachNamespacePauseDone(struct spdk_nvmf_subsystem* subsystem, void* arg, int status)
{
    struct EventContext* ctx = (struct EventContext*)arg;
    if (status == NvmfCallbackStatus::SUCCESS)
    {
        int ret = 0;
        struct spdk_bdev* bdev = NULL;
        char* bdevName = (char*)ctx->eventArg1;
        char* nsid = (char*)ctx->eventArg2;
        bdev = spdk_bdev_get_by_name(bdevName);
        if (bdev)
        {
            struct spdk_nvmf_ns_opts opt;
            uint32_t newNsid = 0;
            memset((char*)&opt, 0, sizeof(struct spdk_nvmf_ns_opts));
            opt.nsid = atoi(nsid);
            newNsid = spdk_nvmf_subsystem_add_ns(subsystem, bdev, &opt, sizeof(opt), NULL);
            free(ctx->eventArg2);
            ctx->eventArg2 = spdk_sprintf_alloc("%u", newNsid);
            if (newNsid > 0)
            {
                SPDK_NOTICELOG("Success to add namespace nsid=%d\n", newNsid);
            }
            else
            {
                SPDK_ERRLOG("fail to add namespace (bdevName=%s)\n", bdevName);
            }
        }
        else
        {
            SPDK_ERRLOG("No bdev with name %s\n", bdevName);
        }

        ret = spdk_nvmf_subsystem_resume(subsystem, AttachNamespaceResumeDone, ctx);
        if (ret != 0)
        {
            SPDK_ERRLOG("fail to resume subsystem(%s) during attach ns\n",
                spdk_nvmf_subsystem_get_nqn(subsystem));
        }
    }
    else
    {
        GenericCallback(__FUNCTION__, arg, NvmfCallbackStatus::FAILED);
        ActivateSubsystem(subsystem);
    }
}

static void
DetachNamespaceResumeDone(struct spdk_nvmf_subsystem* subsystem, void* arg, int status)
{
    if (status != NvmfCallbackStatus::SUCCESS)
    {
        SPDK_ERRLOG("Failed to resume subsystem(%s)\n",
            spdk_nvmf_subsystem_get_nqn(subsystem));
    }

    struct EventContext* ctx = (struct EventContext*)arg;
    uint32_t nsid = 0;
    sscanf((char*)ctx->eventArg1, "%u", &nsid);
    if (spdk_nvmf_subsystem_get_ns(subsystem, nsid) == nullptr)
    {
        status = NvmfCallbackStatus::SUCCESS;
    }
    else
    {
        status = NvmfCallbackStatus::FAILED;
    }
    GenericCallback(__FUNCTION__, arg, status);
}

static void
DetachNamespacePauseDone(struct spdk_nvmf_subsystem* subsystem, void* arg, int status)
{
    struct EventContext* ctx = (struct EventContext*)arg;
    if (status == NvmfCallbackStatus::SUCCESS)
    {
        int ret = 0;
        uint32_t nsid = 0;
        sscanf((char*)ctx->eventArg1, "%u", &nsid);
        ret = spdk_nvmf_subsystem_remove_ns(subsystem, nsid);
        if (ret < 0)
        {
            SPDK_ERRLOG("Failed to detach namespace from subsystem(%s) nsid=%d\n",
                spdk_nvmf_subsystem_get_nqn(subsystem), nsid);
        }
        ret = spdk_nvmf_subsystem_resume(subsystem, DetachNamespaceResumeDone, arg);
        if (ret != 0)
        {
            SPDK_ERRLOG("fail to resume subsystem(%s) during detach ns\n",
                spdk_nvmf_subsystem_get_nqn(subsystem));
        }
    }
    else
    {
        GenericCallback(__FUNCTION__, arg, NvmfCallbackStatus::FAILED);
        ActivateSubsystem(subsystem);
    }
}

static void
DetachNamespaceAllResumeDone(struct spdk_nvmf_subsystem* subsystem, void* arg, int status)
{
    if (status != NvmfCallbackStatus::SUCCESS)
    {
        SPDK_ERRLOG("Failed to resume subsystem(%s)\n",
            spdk_nvmf_subsystem_get_nqn(subsystem));
        status = NvmfCallbackStatus::PARTIAL_FAILED;
    }
    GenericCallback(__FUNCTION__, arg, status);
}

static void
DetachNamespaceAllPauseDone(struct spdk_nvmf_subsystem* subsystem,
    void* arg, int status)
{
    int result = 0;
    struct EventContext* ctx = (struct EventContext*)arg;
    if (status == NvmfCallbackStatus::SUCCESS)
    {
        NvmfTarget target;
        volumeListInfo volsInfo = *(static_cast<volumeListInfo*>(ctx->userArg));
        struct spdk_nvmf_ns* ns = nullptr;
        uint32_t nsid = 0;
        for (auto volId : volsInfo.vols)
        {
            string bdevName = target.GetBdevName(volId, volsInfo.arrayName);
            ns = target.GetNamespace(subsystem, bdevName);
            nsid = spdk_nvmf_ns_get_id(ns);
            int ret = spdk_nvmf_subsystem_remove_ns(subsystem, nsid);
            if (ret < 0)
            {
                SPDK_ERRLOG("Failed to remove subsystem(%s) nsid = %d\n",
                    spdk_nvmf_subsystem_get_nqn(subsystem), nsid);
            }
        }
        result = spdk_nvmf_subsystem_resume(subsystem, DetachNamespaceAllResumeDone, arg);
        if (result != 0)
        {
            SPDK_ERRLOG("Fail to resume subsystem(%s) during detach all ns\n",
                spdk_nvmf_subsystem_get_nqn(subsystem));
        }
        return;
    }
    else
    {
        GenericCallback(__FUNCTION__, arg, NvmfCallbackStatus::FAILED);
        ActivateSubsystem(subsystem);
    }
}

static void
ListenDoneCallback(void* arg, int status)
{
    // This will for adjusting function api (spdk_nvmf_subsystem_add_listener)
}

static void
TargetListenDone(void* arg, int status)
{
    struct EventContext* ctx = (struct EventContext*)arg;
    struct spdk_nvme_transport_id* trid = (struct spdk_nvme_transport_id*)ctx->eventArg1;
    struct spdk_nvmf_subsystem* subsystem = (struct spdk_nvmf_subsystem*)ctx->eventArg2;
    int ret = 0;
    if (status == NvmfCallbackStatus::SUCCESS)
    {
        spdk_nvmf_subsystem_add_listener(subsystem, trid,
                     ListenDoneCallback, nullptr);
    }
    else
    {
        SPDK_ERRLOG("fail to add listener %s status=%d\n", __FUNCTION__, status);
    }

    ctx->eventArg2 = NULL;
    ret = spdk_nvmf_subsystem_resume(subsystem, SubsystemResumeDone, ctx);
    if (ret != 0)
    {
        SPDK_ERRLOG("fail to resume subsystem(%s) during attach listener \n",
            spdk_nvmf_subsystem_get_nqn(subsystem));
    }
}

static void
AttachListenerPauseDone(struct spdk_nvmf_subsystem* subsystem, void* arg, int status)
{
    struct EventContext* ctx = (struct EventContext*)arg;
    if (status == NvmfCallbackStatus::SUCCESS)
    {
        struct spdk_nvme_transport_id* trid = (struct spdk_nvme_transport_id*)ctx->eventArg1;
        ctx->eventArg2 = subsystem;
        spdk_nvmf_tgt_listen(g_spdk_nvmf_tgt, trid);
    }
    else
    {
        GenericCallback(__FUNCTION__, arg, NvmfCallbackStatus::FAILED);
        ActivateSubsystem(subsystem);
    }
}

static void
DetachListenerPauseDone(struct spdk_nvmf_subsystem* subsystem, void* arg, int status)
{
    struct EventContext* ctx = (struct EventContext*)arg;
    if (status == NvmfCallbackStatus::SUCCESS)
    {
        struct spdk_nvme_transport_id* trid = (struct spdk_nvme_transport_id*)ctx->eventArg1;
        int ret = 0;
        ret = spdk_nvmf_subsystem_remove_listener(subsystem, trid);
        if (ret < 0)
        {
            SPDK_ERRLOG("fail to remove listener : %s \n", __FUNCTION__);
        }

        ret = spdk_nvmf_subsystem_resume(subsystem, SubsystemResumeDone, ctx);
        if (ret != 0)
        {
            SPDK_ERRLOG("fail to resume subsystem(%s) during detach listener \n",
                spdk_nvmf_subsystem_get_nqn(subsystem));
        }
    }
    else
    {
        GenericCallback(__FUNCTION__, arg, NvmfCallbackStatus::FAILED);
        ActivateSubsystem(subsystem);
    }
}

void
InitNvmfCallbacks(struct NvmfTargetCallbacks* nvmfCallbacks)
{
    if (nvmfCallbacks)
    {
        nvmfCallbacks->createPosBdevDone = CreatePosBdevDone;
        nvmfCallbacks->deletePosBdevDone = DeletePosBdevDone;
        nvmfCallbacks->createTransportDone = CreateTransportDone;
        nvmfCallbacks->subsystemStartDone = SubsystemStartDone;
        nvmfCallbacks->subsystemStopDone = SubsystemStopDone;
        nvmfCallbacks->subsystemResumeDone = SubsystemResumeDone;
        nvmfCallbacks->attachNamespacePauseDone = AttachNamespacePauseDone;
        nvmfCallbacks->attachNamespaceResumeDone = AttachNamespaceResumeDone;
        nvmfCallbacks->detachNamespacePauseDone = DetachNamespacePauseDone;
        nvmfCallbacks->detachNamespaceResumeDone = DetachNamespaceResumeDone;
        nvmfCallbacks->detachNamespaceAllPauseDone = DetachNamespaceAllPauseDone;
        nvmfCallbacks->detachNamespaceAllResumeDone = DetachNamespaceAllResumeDone;
        nvmfCallbacks->attachListenerPauseDone = AttachListenerPauseDone;
        nvmfCallbacks->targetListenDone = TargetListenDone;
        nvmfCallbacks->detachListenerPauseDone = DetachListenerPauseDone;
    }
}

} // namespace pos

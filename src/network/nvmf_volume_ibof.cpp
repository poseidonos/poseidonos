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

#include "src/network/nvmf_volume_ibof.hpp"

#include <map>
#include <string>

#include "spdk/event.h"
#include "src/device/event_framework_api.h"
#include "src/network/volume_unmount_complete_handler.h"
#include "src/scheduler/event_argument.h"
#include "src/volume/volume_manager.h"

#if defined QOS_ENABLED_FE
#include "src/qos/qos_manager.h"
#endif

namespace ibofos
{
NvmfTarget NvmfVolumeIbof::target;

std::atomic<uint32_t> NvmfVolumeIbof::ioHandlerUnregistered;
std::atomic<bool> NvmfVolumeIbof::detachFailed;

NvmfVolumeIbof::NvmfVolumeIbof(void)
{
}

NvmfVolumeIbof::~NvmfVolumeIbof(void)
{
}

void
NvmfVolumeIbof::_CompleteVolumeUnmount(struct ibof_volume_info* vInfo)
{
    EventSmartPtr event(new VolumeUnmountCompleteHandler(vInfo));
    if (nullptr != event)
    {
        EventArgument::GetEventScheduler()->EnqueueEvent(event);
    }
    else
    {
        SPDK_ERRLOG("Could not allocate VolumeUnmountCompleteHandler\n");
        if (vInfo)
        {
            delete vInfo;
        }
        detachFailed = true;
    }
}

void
NvmfVolumeIbof::_NamespaceDetachedHandler(void* cbArg, int status)
{
    struct ibof_volume_info* vInfo = (struct ibof_volume_info*)cbArg;
    if (status == NvmfCallbackStatus::SUCCESS)
    {
        if (vInfo)
        {
            _CompleteVolumeUnmount(vInfo);
        }
    }
    else
    {
        SPDK_ERRLOG("Could not detach volume\n");
        if (vInfo)
        {
            delete vInfo;
        }
        detachFailed = true;
    }
}

void
NvmfVolumeIbof::_NamespaceDetachedAllHandler(void* cbArg, int status)
{
    int failedVolCount = 0;
    if (status == NvmfCallbackStatus::SUCCESS ||
        status == NvmfCallbackStatus::PARTIAL_FAILED)
    {
        volumeListInfo volsInfo = *(static_cast<volumeListInfo*>(cbArg));
        string subnqn = volsInfo.subnqn;
        vector<int> volList = volsInfo.vols;

        for (auto volId : volList)
        {
            string bdevName = target.GetBdevName(volId);
            struct spdk_nvmf_subsystem* subsystem = target.FindSubsystem(subnqn);
            struct spdk_nvmf_ns* ns = target.GetNamespace(subsystem, bdevName);
            if (ns != nullptr)
            {
                SPDK_NOTICELOG("Requested volume (id=%d) is still attached\n", volId);
                failedVolCount++;
                continue;
            }
            struct ibof_volume_info* vInfo = new ibof_volume_info;
            if (vInfo)
            {
                vInfo->id = volId;
                _CompleteVolumeUnmount(vInfo);
            }
        }
        if (failedVolCount > 0)
        {
            detachFailed = true;
            SPDK_ERRLOG("Failed to Detach All Volumes in Subsystem(%s)\n",
                subnqn.c_str());
            SPDK_NOTICELOG("Only %d out of %lu got detached. Retry Volume Detach\n",
                failedVolCount, volList.size());
        }
    }
    else
    {
        detachFailed = true;
        SPDK_ERRLOG("Cannot detach all volumes\n");
    }
    delete (static_cast<volumeListInfo*>(cbArg));
}

void
NvmfVolumeIbof::_VolumeCreateHandler(void* arg1, void* arg2)
{
    struct ibof_volume_info* vInfo = (struct ibof_volume_info*)arg1;
    if (vInfo)
    {
        string bdevName = target.GetBdevName(vInfo->id);
        target.CreateIBoFBdev(bdevName, vInfo->id, vInfo->size_mb, 512, false);
        delete vInfo;
    }
}

void
NvmfVolumeIbof::VolumeCreated(struct ibof_volume_info* vInfo)
{
    EventFrameworkApi::SendSpdkEvent(EventFrameworkApi::GetFirstReactor(),
        _VolumeCreateHandler, vInfo, nullptr);
}

void
NvmfVolumeIbof::_VolumeDeleteHandler(void* arg1, void* arg2)
{
    struct ibof_volume_info* vInfo = (struct ibof_volume_info*)arg1;
    if (vInfo)
    {
        string bdevName = target.GetBdevName(vInfo->id);
        target.DeleteIBoFBdev(bdevName);
        delete vInfo;
    }
}

void
NvmfVolumeIbof::VolumeDeleted(struct ibof_volume_info* vInfo)
{
    EventFrameworkApi::SendSpdkEvent(EventFrameworkApi::GetFirstReactor(),
        _VolumeDeleteHandler, vInfo, nullptr);
}

void
NvmfVolumeIbof::_VolumeMountHandler(void* arg1, void* arg2)
{
    struct ibof_volume_info* vInfo = (struct ibof_volume_info*)arg1;

    if (vInfo)
    {
        string subNqn(vInfo->nqn);
        string bdevName = target.GetBdevName(vInfo->id);
#if defined QOS_ENABLED_FE
        vInfo->nqn_id = target.GetVolumeNqnId(subsystem);
        QosManagerSingleton::Instance()->UpdateSubsystemToVolumeMap(vInfo->nqn_id, vInfo->id);
        spdk_bdev_ibof_register_io_handler(bdevName.c_str(), vInfo->unvmf_io,
            subNqn.c_str(), vInfo->nqn_id);
#else
        spdk_bdev_ibof_register_io_handler(bdevName.c_str(), vInfo->unvmf_io, subNqn.c_str());
#endif
        target.SetVolumeQos(bdevName, vInfo->iops_limit, vInfo->bw_limit);
        delete vInfo;
    }
}

void
NvmfVolumeIbof::VolumeMounted(struct ibof_volume_info* vInfo)
{
    vInfo->unvmf_io = GetuNVMfIOHandler();
    EventFrameworkApi::SendSpdkEvent(EventFrameworkApi::GetFirstReactor(),
        _VolumeMountHandler, vInfo, nullptr);
}

void
NvmfVolumeIbof::_VolumeUnmountHandler(void* arg1, void* arg2)
{
    struct ibof_volume_info* vInfo = (struct ibof_volume_info*)arg1;
    bool ret = false;
    if (vInfo)
    {
        string bdevName = target.GetBdevName(vInfo->id);
        const char* nqn = get_attached_subsystem_nqn(bdevName.c_str());
        if (nqn != nullptr)
        {
            string subnqn(nqn);
            ret = target.DetachNamespace(subnqn, 0, _NamespaceDetachedHandler, vInfo);
        }
    }
    if (ret == false)
    {
        detachFailed = true;
        if (vInfo)
        {
            delete vInfo;
        }
    }
}

void
NvmfVolumeIbof::VolumeUnmounted(struct ibof_volume_info* vInfo)
{
    EventFrameworkApi::SendSpdkEvent(EventFrameworkApi::GetFirstReactor(),
        _VolumeUnmountHandler, vInfo, nullptr);

    WaitAllIoHandlerUnregistered(1);
}

void
NvmfVolumeIbof::_VolumeUpdateHandler(void* arg1, void* arg2)
{
    struct ibof_volume_info* vInfo = (struct ibof_volume_info*)arg1;
    if (vInfo)
    {
        string bdevName = target.GetBdevName(vInfo->id);
        target.SetVolumeQos(bdevName, vInfo->iops_limit, vInfo->bw_limit);
        delete vInfo;
    }
}

void
NvmfVolumeIbof::VolumeUpdated(struct ibof_volume_info* vInfo)
{
    EventFrameworkApi::SendSpdkEvent(EventFrameworkApi::GetFirstReactor(),
        _VolumeUpdateHandler, vInfo, nullptr);
}

void
NvmfVolumeIbof::_VolumeDetachHandler(void* volListInfo, void* arg)
{
    int ret = false;
    volumeListInfo volsInfo = *(static_cast<volumeListInfo*>(volListInfo));
    string subnqn = volsInfo.subnqn;
    ret = target.DetachNamespaceAll(subnqn, _NamespaceDetachedAllHandler, volListInfo);
    if (ret == false)
    {
        detachFailed = true;
        delete (static_cast<volumeListInfo*>(volListInfo));
    }
}

void
NvmfVolumeIbof::VolumeDetached(vector<int>& volList)
{
    uint32_t volsCountToDetach = 0;
    map<string, vector<int>> volsPerSubsystem;
    for (auto volId : volList)
    {
        string bdevName = target.GetBdevName(volId);
        const char* nqn = get_attached_subsystem_nqn(bdevName.c_str());
        if (nqn != nullptr)
        {
            string subnqn(nqn);
            if (subnqn.empty() == false)
            {
                volsPerSubsystem[subnqn].push_back(volId);
                volsCountToDetach++;
            }
        }
    }
    if (volsCountToDetach == 0)
    {
        return;
    }
    uint32_t target_core = EventFrameworkApi::GetFirstReactor();
    for (auto volumes : volsPerSubsystem)
    {
        volumeListInfo* volsInfo = new volumeListInfo;
        volsInfo->subnqn = volumes.first;
        volsInfo->vols = volumes.second;

        EventFrameworkApi::SendSpdkEvent(target_core, _VolumeDetachHandler,
            volsInfo, nullptr);
    }
    WaitAllIoHandlerUnregistered(volsCountToDetach);
}

void
NvmfVolumeIbof::AddUnregisteredIoHandlerCnt(void)
{
    ioHandlerUnregistered++;
}

void
NvmfVolumeIbof::WaitAllIoHandlerUnregistered(uint32_t volCnt)
{
    while (volCnt != ioHandlerUnregistered)
    {
        if (detachFailed == true)
        {
            SPDK_ERRLOG("Detach Volume Failed. No need to wait for volume unmount");
            break;
        }
        usleep(1);
    }
    ioHandlerUnregistered = 0;
    detachFailed = false;
}
} // namespace ibofos

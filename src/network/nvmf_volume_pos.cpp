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

#include "src/network/nvmf_volume_pos.h"

#include <map>
#include <string>

#include "spdk/event.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/lib/system_timeout_checker.h"
#include "src/qos/qos_manager.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/volume/volume_manager.h"

namespace pos
{
NvmfTarget NvmfVolumePos::target;

std::atomic<bool> NvmfVolumePos::detachFailed;
std::atomic<uint32_t> NvmfVolumePos::volumeDetachedCnt;

NvmfVolumePos::NvmfVolumePos(unvmf_io_handler ioHandler)
: ioHandler(ioHandler)
{
}

NvmfVolumePos::~NvmfVolumePos(void)
{
}

void
NvmfVolumePos::_NamespaceDetachedHandler(void* cbArg, int status)
{
    struct pos_volume_info* vInfo = (struct pos_volume_info*)cbArg;
    if (status == NvmfCallbackStatus::SUCCESS)
    {
        if (vInfo)
        {
            volumeDetachedCnt++;
            string bdevName = target.GetBdevName(vInfo->id, vInfo->array_name);
            spdk_bdev_pos_unregister_io_handler(bdevName.c_str());
            reset_pos_volume_info(bdevName.c_str());
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
NvmfVolumePos::_NamespaceDetachedAllHandler(void* cbArg, int status)
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
            string bdevName = target.GetBdevName(volId, volsInfo.arrayName);
            struct spdk_nvmf_subsystem* subsystem = target.FindSubsystem(subnqn);
            struct spdk_nvmf_ns* ns = target.GetNamespace(subsystem, bdevName);
            if (ns != nullptr)
            {
                SPDK_NOTICELOG("Requested volume(%s) is still attached\n", bdevName.c_str());
                failedVolCount++;
                continue;
            }
            volumeDetachedCnt++;
            spdk_bdev_pos_unregister_io_handler(bdevName.c_str());
            reset_pos_volume_info(bdevName.c_str());
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
NvmfVolumePos::_VolumeCreateHandler(void* arg1, void* arg2)
{
    struct pos_volume_info* vInfo = (struct pos_volume_info*)arg1;
    if (vInfo)
    {
        string bdevName = target.GetBdevName(vInfo->id, vInfo->array_name);
        target.CreatePosBdev(bdevName, vInfo->id, vInfo->size_mb, 512, false, vInfo->array_name, vInfo->array_id);
        delete vInfo;
    }
}

void
NvmfVolumePos::VolumeCreated(struct pos_volume_info* vInfo)
{
    EventFrameworkApiSingleton::Instance()->SendSpdkEvent(EventFrameworkApiSingleton::Instance()->GetFirstReactor(),
        _VolumeCreateHandler, vInfo, nullptr);
}

void
NvmfVolumePos::_VolumeDeleteHandler(void* arg1, void* arg2)
{
    struct pos_volume_info* vInfo = (struct pos_volume_info*)arg1;
    if (vInfo)
    {
        string bdevName = target.GetBdevName(vInfo->id, vInfo->array_name);
        target.DeletePosBdev(bdevName);
        delete vInfo;
    }
}

void
NvmfVolumePos::VolumeDeleted(struct pos_volume_info* vInfo)
{
    EventFrameworkApiSingleton::Instance()->SendSpdkEvent(EventFrameworkApiSingleton::Instance()->GetFirstReactor(),
        _VolumeDeleteHandler, vInfo, nullptr);
}

void
NvmfVolumePos::_VolumeMountHandler(void* arg1, void* arg2)
{
    struct pos_volume_info* vInfo = (struct pos_volume_info*)arg1;

    if (vInfo)
    {
        string subNqn(vInfo->nqn);
        string bdevName = target.GetBdevName(vInfo->id, vInfo->array_name);
        spdk_bdev_pos_register_io_handler(bdevName.c_str(), vInfo->unvmf_io);
        int32_t nqn_id = target.GetVolumeNqnId(subNqn);
        if (nqn_id < 0)
        {
            SPDK_ERRLOG("Failed to Get Subsystem Id. Unable to update subsystem to volume map and volume info.\n");
            delete vInfo;
            return;
        }
        QosManagerSingleton::Instance()->UpdateSubsystemToVolumeMap(nqn_id, vInfo->id);
        set_pos_volume_info(bdevName.c_str(), subNqn.c_str(), nqn_id);
        target.SetVolumeQos(bdevName, vInfo->iops_limit, vInfo->bw_limit);
        delete vInfo;
    }
}

void
NvmfVolumePos::VolumeMounted(struct pos_volume_info* vInfo)
{
    vInfo->unvmf_io = ioHandler;
    EventFrameworkApiSingleton::Instance()->SendSpdkEvent(EventFrameworkApiSingleton::Instance()->GetFirstReactor(),
        _VolumeMountHandler, vInfo, nullptr);
}

void
NvmfVolumePos::_VolumeUnmountHandler(void* arg1, void* arg2)
{
    struct pos_volume_info* vInfo = (struct pos_volume_info*)arg1;
    bool ret = false;
    if (vInfo)
    {
        string bdevName = target.GetBdevName(vInfo->id, vInfo->array_name);
        const char* nqn = get_attached_subsystem_nqn(bdevName.c_str());
        string subnqn(nqn);
        ret = target.DetachNamespace(subnqn, 0, _NamespaceDetachedHandler, vInfo);
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
NvmfVolumePos::VolumeUnmounted(struct pos_volume_info* vInfo)
{
    bool mounted = target.CheckVolumeAttached(vInfo->id, vInfo->array_name);
    if (mounted == false)
    {
        volumeDetachedCnt++;
    }
    else
    {
        volumeDetachedCnt = 0;
        EventFrameworkApiSingleton::Instance()->SendSpdkEvent(EventFrameworkApiSingleton::Instance()->GetFirstReactor(),
            _VolumeUnmountHandler, vInfo, nullptr);
    }
}

void
NvmfVolumePos::_VolumeUpdateHandler(void* arg1, void* arg2)
{
    struct pos_volume_info* vInfo = (struct pos_volume_info*)arg1;
    if (vInfo)
    {
        string bdevName = target.GetBdevName(vInfo->id, vInfo->array_name);
        target.SetVolumeQos(bdevName, vInfo->iops_limit, vInfo->bw_limit);
        delete vInfo;
    }
}

void
NvmfVolumePos::VolumeUpdated(struct pos_volume_info* vInfo)
{
    EventFrameworkApiSingleton::Instance()->SendSpdkEvent(EventFrameworkApiSingleton::Instance()->GetFirstReactor(),
        _VolumeUpdateHandler, vInfo, nullptr);
}

void
NvmfVolumePos::_VolumeDetachHandler(void* volListInfo, void* arg)
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
NvmfVolumePos::VolumeDetached(vector<int>& volList, string arrayName)
{
    uint32_t volsCountToDetach = 0;
    map<string, vector<int>> volsPerSubsystem;
    for (auto volId : volList)
    {
        string bdevName = target.GetBdevName(volId, arrayName);
        const char* nqn = get_attached_subsystem_nqn(bdevName.c_str());
        if (nqn != nullptr)
        {
            string subnqn(nqn);
            if (subnqn.empty() == false && target.CheckVolumeAttached(volId, arrayName) == true)
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
    uint32_t target_core = EventFrameworkApiSingleton::Instance()->GetFirstReactor();
    for (auto volumes : volsPerSubsystem)
    {
        volumeListInfo* volsInfo = new volumeListInfo;
        volsInfo->subnqn = volumes.first;
        volsInfo->vols = volumes.second;
        volsInfo->arrayName = arrayName;
        EventFrameworkApiSingleton::Instance()->SendSpdkEvent(target_core, _VolumeDetachHandler,
            volsInfo, nullptr);
    }
}

bool
NvmfVolumePos::WaitRequestedVolumesDetached(uint32_t volCnt)
{
    SystemTimeoutChecker timeChecker;
    uint64_t time = 15 * 1000000000ULL;
    timeChecker.SetTimeout(time);
    while ((volumeDetachedCnt < volCnt))
    {
        if ((true == detachFailed) || (true == timeChecker.CheckTimeout()))
        {
            volumeDetachedCnt = 0;
            detachFailed = false;
            return false;
        }
        usleep(1);
    }
    volumeDetachedCnt = 0;
    detachFailed = false;
    return true;
}

} // namespace pos

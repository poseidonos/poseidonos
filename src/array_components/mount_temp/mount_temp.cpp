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

#include "mount_temp.h"

#include "src/device/device_manager.h"
#include "src/io/frontend_io/flush_command_manager.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/logger/logger.h"
#include "src/volume/volume_manager.h"
#include "src/array_components/mount_temp/debug_info_updater.h"
#include "src/debug/debug_info.h"
#include "src/io/frontend_io/unvmf_io_handler.h"
#include "src/array_mgmt/array_manager.h"
#include "src/array/interface/i_abr_control.h"
#if defined QOS_ENABLED_BE
#include "src/qos/qos_manager.h"
#endif
#include "src/sys_info/space_info.h"
#ifdef _ADMIN_ENABLED
#include "src/admin/smart_log_mgr.h"
#endif
namespace pos
{
MountTemp::MountTemp(IAbrControl* abr, string name)
{
    abrControl = abr;
    arrayName = name;
}

int
MountTemp::Unmount2(void)
{
    // after array component dispose
    int ret = 0;
    int eventId = 9999;

#ifdef _ADMIN_ENABLED
    POS_TRACE_INFO(eventId, "start smart log store");
    SmartLogMgrSingleton::Instance()->StoreLogData();
    SmartLogMgrSingleton::ResetInstance();
#endif
    POS_TRACE_INFO(eventId, "start flush cmd manager reset instance");
    FlushCmdManagerSingleton::ResetInstance();
    POS_TRACE_INFO(eventId, "start meta file system mgr unmount");
    metaFs.mgmt.UnmountSystem(arrayName);

#if defined QOS_ENABLED_BE
    POS_TRACE_INFO(eventId, "start qos manager reset instance");
    QosManagerSingleton::ResetInstance();
#endif

    _ResetNvmf();

    return ret;
}

int
MountTemp::Mount1(void)
{
    debugInfoUpdater->Update();

    int ret = 0;
    ret = _MountMetaFilesystem();
    if (0 > ret)
    {
        return ret;
    }
    _InitNvmf();

#if defined QOS_ENABLED_BE
    QosManagerSingleton::Instance()->Initialize();
#endif

    return ret;
}

int
MountTemp::_MountMetaFilesystem(void)
{
    bool isInitialized = abrControl->GetMfsInit(arrayName);

    MetaStorageMediaInfoList mediaList;
    _RegisterMediaInfoIfAvailable(META_NVM, mediaList);
    _RegisterMediaInfoIfAvailable(META_SSD, mediaList);

    bool isSuccess = metaFs.Init(arrayName, mediaList);
    if (!isSuccess)
    {
        return -1;
        // FIXME: handle error
    }

    MetaFsReturnCode<POS_EVENT_ID> sysRC;
    if (false == isInitialized) // hardly know whether valid filesystem exists or not, if the filesystem has been established once before
    {
        sysRC = metaFs.mgmt.CreateSystem(arrayName);
        if (sysRC.IsSuccess())
        {
            int ret = 0;
            ret = abrControl->SetMfsInit(arrayName, true);
            if (0 != ret)
            {
                return ret;
            }
        }
        else
        {
            return -1;
            // FIXME: need to report filesystem mount error to host
        }
    }
    sysRC = metaFs.mgmt.MountSystem(arrayName);
    if (!sysRC.IsSuccess())
    {
        return -1;
        // if (sysRC.sc == MetaFsStatusCodeFsControlSpcf::FileSysNotFound)
        // {
        // }
    }
    return 0;
}

void
MountTemp::_RegisterMediaInfoIfAvailable(
    PartitionType ptnType, MetaStorageMediaInfoList& mediaList)
{
    MetaStorageInfo media = _MakeMetaStorageMediaInfo(ptnType);
    mediaList.push_back(media);
}

MetaStorageInfo
MountTemp::_MakeMetaStorageMediaInfo(PartitionType ptnType)
{
    MetaStorageInfo newInfo;
    switch (ptnType)
    {
        case META_NVM:
        {
            newInfo.media = MetaStorageType::NVRAM;
        }
        break;
        case META_SSD:
        {
            newInfo.media = MetaStorageType::SSD;
        }
        break;
        default:
        {
            assert(false);
        }
    }

    IArrayInfo* info = ArrayMgr::Instance()->GetArrayInfo(arrayName);
    const PartitionLogicalSize* ptnSize = info->GetSizeInfo(ptnType);
    newInfo.mediaCapacity = static_cast<uint64_t>(ptnSize->totalStripes) * ptnSize->blksPerStripe * ArrayConfig::BLOCK_SIZE_BYTE;
    return newInfo;
}

void
MountTemp::_InitNvmf(void)
{
    nvmfVolume = new pos::NvmfVolumeIbof();
    unvmf_io_handler handler = {.submit = UNVMfSubmitHandler,
        .complete = UNVMfCompleteHandler};
    nvmfVolume->SetuNVMfIOHandler(handler);
    nvmfTargetEventSubscriber = new pos::NvmfTargetEventSubscriber(nvmfVolume, arrayName);
}

void
MountTemp::_ResetNvmf(void)
{
    if (nvmfTargetEventSubscriber != nullptr)
    {
        delete nvmfTargetEventSubscriber;
        nvmfTargetEventSubscriber = nullptr;
    }

    if (nvmfVolume != nullptr)
    {
        delete nvmfVolume;
        nvmfVolume = nullptr;
    }
}

} // namespace pos

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

#include "main_handler.h"

#include "src/allocator/allocator.h"
#include "src/debug/debug_info.h"
#include "src/device/device_manager.h"
#include "src/io/general_io/command_timeout_handler.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/journal_manager/journal_manager.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "src/master_context/mbr_manager.h"
#include "src/scheduler/scheduler_api.h"
#include "src/volume/volume_manager.h"

#if defined QOS_ENABLED_BE
#include "src/qos/qos_manager.h"
#endif
#include "src/gc/garbage_collector.h"
#include "src/sys_info/space_info.h"
#if defined NVMe_FLUSH_HANDLING
#include "src/io/frontend_io/flush_command_manager.h"
#endif

namespace ibofos
{
MainHandler::MainHandler(void)
: StateEvent("main_handler")
{
    StateManagerSingleton::Instance()->Subscribe(this);
}

IbofosInfo
MainHandler::GetInfo(void)
{
    IbofosInfo info;
    info.state = StateManagerSingleton::Instance()->GetStateStr();
    info.situation = StateManagerSingleton::Instance()->GetSituationStr();
    info.rebuildingProgress = ArraySingleton::Instance()->GetRebuildingProgress();
    info.totalCapacity = SpaceInfo::SystemCapacity();
    info.usedCapacity = SpaceInfo::Used();
    return info;
}

void
MainHandler::_WaitState(StateContext& goal)
{
    std::unique_lock<std::mutex> lock(mtx);
    while (currState != goal)
    {
        cv.wait(lock);
    }
}

void
MainHandler::_WaitStateFor(StateContext& goal, uint32_t sec)
{
    std::unique_lock<std::mutex> lock(mtx);
    while (currState != goal)
    {
        if (cv.wait_for(lock, chrono::seconds(sec)) == cv_status::timeout)
        {
            break;
        }
    }
}

void
MainHandler::_PrepareEventSubscription(void)
{
    // Registering order is important.
    nvmfTargetEventSubscriber->RegisterToPublisher();
    RbaStateManagerSingleton::Instance()->RegisterToPublisher();
    MapperSingleton::Instance()->RegisterToPublisher();
    AllocatorSingleton::Instance()->RegisterToPublisher();
#if defined QOS_ENABLED_BE
    QosManagerSingleton::Instance()->RegisterToPublisher();
#endif
}

void
MainHandler::_ReleaseEventSubscription(void)
{
#if defined QOS_ENABLED_BE
    QosManagerSingleton::Instance()->RemoveFromPublisher();
#endif
    AllocatorSingleton::Instance()->RemoveFromPublisher();
    MapperSingleton::Instance()->RemoveFromPublisher();
    RbaStateManagerSingleton::Instance()->RemoveFromPublisher();
    nvmfTargetEventSubscriber->RemoveFromPublisher();
}

int
MainHandler::Unmount(void)
{
    int ret = 0;
    if (ibofMounted)
    {
        StateManager* stateMgr = StateManagerSingleton::Instance();
        if (stateMgr->ExistRebuild())
        {
            int eventId = (int)IBOF_EVENT_ID::SYSTEM_UNMOUNT_PRIORITY_ERROR;
            IBOF_TRACE_ERROR(eventId, "Failed to unmount system. Rebuild is not finished");
            ret = eventId;
            return ret;
        }
        unmountCtx = stateMgr->Invoke(sender, Situation::TRY_UNMOUNT);
        CommandTimeoutHandlerSingleton::Instance()->DisableAbort();
        uint32_t waitTimeout = 2;
        _WaitStateFor(unmountCtx, waitTimeout);

        if (currState != unmountCtx && currState.GetState() != State::STOP)
        {
            int eventId = (int)IBOF_EVENT_ID::SYSTEM_UNMOUNT_PRIORITY_ERROR;
            IBOF_TRACE_ERROR(eventId, "Failed to unmount system. Higher priority jobs are progressing.");
            ret = eventId;
            stateMgr->Remove(unmountCtx);
        }
        else
        {
            int eventId = (int)IBOF_EVENT_ID::SYSTEM_UNMOUNTING;
            IBOF_TRACE_INFO(eventId, "start volume dispose");
            VolumeManagerSingleton::Instance()->Dispose();

            IBOF_TRACE_INFO(eventId, "start garbage collector end");
            GarbageCollectorSingleton::Instance()->End();

            IBOF_TRACE_INFO(eventId, "start flush all user data");
            AllocatorSingleton::Instance()->FlushAllUserdata();

            IBOF_TRACE_INFO(eventId, "start mapper async store");
            MapperSingleton::Instance()->AsyncStore();

            IBOF_TRACE_INFO(eventId, "start allocator store");
            AllocatorSingleton::Instance()->Store();

            IBOF_TRACE_INFO(eventId, "start checking map store done");
            MapperSingleton::Instance()->CheckMapStoreDone();

            IBOF_TRACE_INFO(eventId, "start journal reset");
            JournalManagerSingleton::Instance()->Reset();

            IBOF_TRACE_INFO(eventId, "start journal reset instance");
            JournalManagerSingleton::ResetInstance();

            IBOF_TRACE_INFO(eventId, "start release event subscription");
            _ReleaseEventSubscription();

            _ResetNvmf();

            IBOF_TRACE_INFO(eventId, "start mapper close");
            MapperSingleton::Instance()->Close();

            IBOF_TRACE_INFO(eventId, "start mapper reset instance");
            MapperSingleton::ResetInstance();

            IBOF_TRACE_INFO(eventId, "start allocator reset instance");
            AllocatorSingleton::ResetInstance();

            IBOF_TRACE_INFO(eventId, "start rba state reset instance");
            RbaStateManagerSingleton::ResetInstance();
#if defined NVMe_FLUSH_HANDLING
            IBOF_TRACE_INFO(eventId, "start flush cmd manager reset instance");
            FlushCmdManagerSingleton::ResetInstance();
#endif
            //    VolumeManager:: //Todo
            IBOF_TRACE_INFO(eventId, "start meta file system mgr unmount");
            metaFsMgr.sys.Unmount();

            IBOF_TRACE_INFO(eventId, "start array unmount");
            ret = ArraySingleton::Instance()->Unmount();
            if (ret == 0)
            {
                IBOF_TRACE_INFO(eventId, "start state manager remove with normalCtx");
                StateManagerSingleton::Instance()->Remove(normalCtx);
                ibofMounted = false;
            }

            IBOF_TRACE_INFO(eventId, "start state manager remove with unmountCtx");
            StateManagerSingleton::Instance()->Remove(unmountCtx);
#if defined QOS_ENABLED_BE
            IBOF_TRACE_INFO(eventId, "start qos manager finalize");
            QosManagerSingleton::Instance()->Finalize();
#endif
        }
    }
    else
    {
        int eventId = (int)IBOF_EVENT_ID::SYSTEM_UNMOUNT_ERROR;
        IBOF_TRACE_ERROR(eventId, "Failed to unmount system. Already unmounted");
        ret = eventId;
    }

    return ret;
}

int
MainHandler::Exit(void)
{
    if (currState.GetState() != State::OFFLINE)
    {
        int eventId = (int)IBOF_EVENT_ID::SYSTEM_ALD_MOUNTED;
        IBOF_TRACE_ERROR(eventId, "Failed to exit system. Already mounted");
        return eventId;
    }
#if defined QOS_ENABLED_BE
    else
    {
        IBOF_TRACE_INFO(EID(SYSTEM_EXITING), "Qos Manager reset instance");
        QosManagerSingleton::ResetInstance();
    }
#endif
    return 0;
}

int
MainHandler::Mount(void)
{
    if (currState.GetState() != State::OFFLINE)
    {
        int eventId = (int)IBOF_EVENT_ID::SYSTEM_ALD_MOUNTED;
        IBOF_TRACE_ERROR(eventId, "Failed to mount system. Already mounted");
        return eventId;
    }

    mountCtx = StateManagerSingleton::Instance()->Invoke(sender, Situation::TRY_MOUNT);
    _WaitState(mountCtx);
    debugInfo->UpdateModulePointer();

    int ret = 0;
    ret = ArraySingleton::Instance()->Mount();
    if (0 != ret)
    {
        StateManagerSingleton::Instance()->Remove(mountCtx);
        return ret;
    }
    ret = _MountMetaFilesystem();
    if (0 > ret)
    {
        StateManagerSingleton::Instance()->Remove(mountCtx);
        return ret;
    }

    _InitNvmf();
    _PrepareEventSubscription();

#if defined QOS_ENABLED_BE
    QosManagerSingleton::Instance()->Initialize();
#endif

    MapperSingleton::Instance()->Init();
    AllocatorSingleton::Instance()->Init();
    VolumeManagerSingleton::Instance()->Initialize();

    ret = JournalManagerSingleton::Instance()->Init();
    if (0 == ret)
    {
        ret = JournalManagerSingleton::Instance()->DoRecovery();
    }
    if (0 > ret)
    {
        StateManagerSingleton::Instance()->Remove(mountCtx);
        return ret;
    }

    GarbageCollectorSingleton::Instance()->Start();

    normalCtx = StateManagerSingleton::Instance()->InvokeAndRemove(mountCtx, sender, Situation::NORMAL);
    _WaitStateFor(normalCtx, 2);
    ibofMounted = true;

    ArraySingleton::Instance()->NotifyIbofosMounted();
    CommandTimeoutHandlerSingleton::Instance()->EnableAbort();
    return ret;
}

int
MainHandler::_MountMetaFilesystem(void)
{
    MbrManager& mbrManager = *MbrManagerSingleton::Instance();
    int arrayNum = 0;
    bool isInitialized = mbrManager.GetMfsInit(arrayNum);

    MetaStorageMediaInfoList mediaList;
    _RegisterMediaInfoIfAvailable(META_NVM, mediaList);
    _RegisterMediaInfoIfAvailable(META_SSD, mediaList);

    bool isSuccess;
    isSuccess = metaFsMgr.Init(mediaList);
    if (!isSuccess)
    {
        return -1;
        // FIXME: handle error
    }

    MetaFsReturnCode<IBOF_EVENT_ID> sysRC;
    if (false == isInitialized) // hardly know whether valid filesystem exists or not, if the filesystem has been established once before
    {
        sysRC = metaFsMgr.sys.Create();
        if (sysRC.IsSuccess())
        {
            int ret = 0;
            int arrayNum = 0;
            ret = mbrManager.SetMfsInit(arrayNum, true);
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
    sysRC = metaFsMgr.sys.Mount();
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
MainHandler::_RegisterMediaInfoIfAvailable(
    PartitionType ptnType, MetaStorageMediaInfoList& mediaList)
{
    MetaStorageMediaInfo media = _MakeMetaStorageMediaInfo(ptnType);
    mediaList.push_back(media);
}

MetaStorageMediaInfo
MainHandler::_MakeMetaStorageMediaInfo(PartitionType ptnType)
{
    MetaStorageMediaInfo newInfo;
    Array& sysArray = *ArraySingleton::Instance();

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
    const PartitionLogicalSize* ptnSize = sysArray.GetSizeInfo(ptnType);
    newInfo.mediaCapacity = static_cast<uint64_t>(ptnSize->totalStripes) * ptnSize->blksPerStripe * ArrayConfig::BLOCK_SIZE_BYTE;
    return newInfo;
}

void
MainHandler::StateChanged(StateContext prev, StateContext next)
{
    std::unique_lock<std::mutex> lock(mtx);
    currState = next;
    cv.notify_all();
}

void
MainHandler::_InitNvmf(void)
{
    nvmfVolume = new ibofos::NvmfVolumeIbof();
    unvmf_io_handler handler = {.submit = UNVMfSubmitHandler,
        .complete = UNVMfCompleteHandler};
    nvmfVolume->SetuNVMfIOHandler(handler);
    nvmfTargetEventSubscriber = new ibofos::NvmfTargetEventSubscriber(nvmfVolume);
}

void
MainHandler::_ResetNvmf(void)
{
    if (nullptr != nvmfTargetEventSubscriber)
    {
        delete nvmfTargetEventSubscriber;
        nvmfTargetEventSubscriber = nullptr;
    }

    if (nullptr != nvmfVolume)
    {
        delete nvmfVolume;
        nvmfVolume = nullptr;
    }
}
} // namespace ibofos

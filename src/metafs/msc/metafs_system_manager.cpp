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

/* 
 * PoseidonOS - Meta File mgmt Layer
 * 
 * Meta File System Manager
*/

#include "metafs_system_manager.h"
#include "metafs_mbr_mgr.h"
#include "meta_io_manager.h"
#include "mfs_state_procedure.h"

#include <string>

namespace pos
{
MetaFsSystemManager mfsSysMgr;
MetaFsSystemManager& mscTopMgr = mfsSysMgr;

MetaFsSystemManager::MetaFsSystemManager(void)
: isMfsUnmounted(false),
  isTheFirst(true),
  mbrMap(mfsStateMgr.GetMetaVolumeMbrMap())
{
    _InitReqHandler();
}

MetaFsSystemManager::~MetaFsSystemManager(void)
{
}

MetaFsSystemManager&
MetaFsSystemManager::GetInstance(void)
{
    return mfsSysMgr;
}

const char*
MetaFsSystemManager::GetModuleName(void)
{
    return "MetaFS Manager";
}

POS_EVENT_ID
MetaFsSystemManager::CheckReqSanity(MetaFsControlReqMsg& reqMsg)
{
    POS_EVENT_ID sc = POS_EVENT_ID::SUCCESS;

    if (false == reqMsg.IsValid())
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "Given request is incorrect. Please check parameters.");
        return POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    return sc;
}

bool
MetaFsSystemManager::Init(std::string& arrayName, MetaStorageMediaInfoList& mediaInfoList)
{
    if (mbrMap.IsMbrLoaded(arrayName))
        return false;

    mbrMap.Init(arrayName, MetaStorageType::SSD, MetaFsMBRManager::FILESYSTEM_MBR_BASE_LPN);

    for (auto& item : mediaInfoList)
    {
        mbrMap.RegisterVolumeGeometry(arrayName, item);
    }

    mfsStateMgr.RequestSystemStateChange(MetaFsSystemState::PowerOn, MetaFsSystemState::Init);
    POS_EVENT_ID sc = mfsStateMgr.ExecuteStateTransition(arrayName);
    if (sc != POS_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MODULE_INIT_FAILED,
            "CoreMgr init failed.");
        return false;
    }

    if (!isTheFirst)
        return true;

    SetModuleInit();

    if (isMfsUnmounted == true)
    {
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "Meta filesystem has been re-initialized without power off...");

        isMfsUnmounted = false;
    }

    return true;
}

void
MetaFsSystemManager::_InitReqHandler(void)
{
    _RegisterReqHandler(MetaFsControlReqType::MountSystem,      &MetaFsSystemManager::_HandleMountReq);
    _RegisterReqHandler(MetaFsControlReqType::UnmountSystem,    &MetaFsSystemManager::_HandleUnmountReq);
    _RegisterReqHandler(MetaFsControlReqType::CreateSystem,     &MetaFsSystemManager::_HandleFileSysCreateReq);
    _RegisterReqHandler(MetaFsControlReqType::AddArray,         &MetaFsSystemManager::_HandleAddArray);
    _RegisterReqHandler(MetaFsControlReqType::RemoveArray,      &MetaFsSystemManager::_HandleRemoveArray);
}

void
MetaFsSystemManager::_RegisterReqHandler(MetaFsControlReqType reqType, MetaFsControlReqHandlerPointer handler)
{
    reqHandler[(uint32_t)reqType] = handler;
}

bool
MetaFsSystemManager::Bringup(std::string& arrayName)
{
    mfsStateMgr.RequestSystemStateChange(MetaFsSystemState::Ready, MetaFsSystemState::Ready);
    POS_EVENT_ID sc = mfsStateMgr.ExecuteStateTransition(arrayName);
    if (sc != POS_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MODULE_BRINGUP_FAILED,
            "CoreMgr bringup failed.");
        return false;
    }

    if (isTheFirst)
    {
        SetModuleReady();
    }

    return true;
}

POS_EVENT_ID
MetaFsSystemManager::ProcessNewReq(MetaFsControlReqMsg& reqMsg)
{
    POS_EVENT_ID sc;
    sc = (this->*(reqHandler[(uint32_t)reqMsg.reqType]))(reqMsg);
    return sc;
}

bool
MetaFsSystemManager::IsMounted(void)
{
    return (MetaFsSystemState::Active == mfsStateMgr.GetCurrSystemState());
}

uint64_t
MetaFsSystemManager::GetEpochSignature(std::string& arrayName)
{
    return mbrMap.GetEpochSignature(arrayName);
}

bool
MetaFsSystemManager::_IsSiblingModuleReady(void)
{
    return true;
}

POS_EVENT_ID
MetaFsSystemManager::_HandleFileSysCreateReq(MetaFsControlReqMsg& reqMsg)
{
    if (mbrMap.IsValidMBRExist(reqMsg.arrayName))
    {
        MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_WARNING_INIT_AGAIN,
            "You attempt to create Meta filesystem again. Can't service this request. Ignore it.");
        return POS_EVENT_ID::SUCCESS;
    }

    mfsStateMgr.RequestSystemStateChange(MetaFsSystemState::Create, MetaFsSystemState::Create);
    POS_EVENT_ID sc = mfsStateMgr.ExecuteStateTransition(reqMsg.arrayName);
    if (sc != POS_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED,
            "Meta filesystem creation request has been failed.");

        return sc;
    }

    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Meta filesystem has been created...");

    return sc;
}

POS_EVENT_ID
MetaFsSystemManager::_HandleMountReq(MetaFsControlReqMsg& reqMsg)
{
    if (MetaFsSystemState::Active != mfsStateMgr.GetCurrSystemState())
    {
        mfsStateMgr.RequestSystemStateChange(MetaFsSystemState::Open, MetaFsSystemState::Active);
        POS_EVENT_ID sc = mfsStateMgr.ExecuteStateTransition(reqMsg.arrayName);
        if (sc != POS_EVENT_ID::SUCCESS)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MEDIA_MOUNT_FAILED,
                "Meta filesystem mount request has been failed.");

            return sc;
        }
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "Meta filesystem has been mounted...");
    }
    else
    {
        MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_SYSTEM_MOUNT_AGAIN,
            "Meta filesystem is already mounted. Ignore it");
    }

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaFsSystemManager::_HandleUnmountReq(MetaFsControlReqMsg& reqMsg)
{
    POS_EVENT_ID sc = POS_EVENT_ID::SUCCESS;
    try
    {
        if (MetaFsSystemState::Shutdown != mfsStateMgr.GetCurrSystemState())
        {
            bool isTheLast = (1 == mbrMap.GetMountedMbrCount()) ? true : false;

            if (!isTheLast)
            {
                bool resetCxt = false;
                if (!mvmTopMgr.Close(resetCxt, reqMsg.arrayName))
                {
                    // Reset MetaFS DRAM Context
                    if (resetCxt == true)
                        return POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED;
                    else
                        return POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED_DUE_TO_ACTIVE_FILE;
                }
                mbrMap.SetPowerStatus(reqMsg.arrayName, true /*NPOR status*/);
                if (true != mbrMap.SaveContent(reqMsg.arrayName))
                    return POS_EVENT_ID::MFS_META_SAVE_FAILED;
                if (POS_EVENT_ID::SUCCESS != metaStorage->Close(reqMsg.arrayName))
                    return POS_EVENT_ID::MFS_META_STORAGE_CLOSE_FAILED;
                mbrMap.Remove(reqMsg.arrayName);

                return POS_EVENT_ID::SUCCESS;
            }

            mfsStateMgr.RequestSystemStateChange(MetaFsSystemState::Quiesce, MetaFsSystemState::Shutdown);
            sc = mfsStateMgr.ExecuteStateTransition(reqMsg.arrayName);
            if (sc != POS_EVENT_ID::SUCCESS)
            {
                // due to both array stop state and active files.
                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MEDIA_UNMOUNT_FAILED,
                    "Meta filesystem unmount request has been failed.");

                throw sc;
            }
            else
            {
                MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
                    "Meta filesystem has been unmounted...");

                throw POS_EVENT_ID::SUCCESS;
            }
        }
        else
        {
            MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_SYSTEM_UNMOUNT_AGAIN,
                "Meta filesystem is already unmounted. Ignore it");

            throw POS_EVENT_ID::SUCCESS;
        }
    }
    catch (POS_EVENT_ID event)
    {
        if (sc != POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED_DUE_TO_ACTIVE_FILE)
        {
            SetModuleInit();
            isMfsUnmounted = true;
        }

        return event;
    }
}

POS_EVENT_ID
MetaFsSystemManager::_HandleAddArray(MetaFsControlReqMsg& reqMsg)
{
    if (true == mimTopMgr.AddArrayInfo(reqMsg.arrayName))
        return POS_EVENT_ID::SUCCESS;
    else
        return POS_EVENT_ID::MFS_ARRAY_ADD_FAILED;
}

POS_EVENT_ID
MetaFsSystemManager::_HandleRemoveArray(MetaFsControlReqMsg& reqMsg)
{
    if (true == mimTopMgr.RemoveArrayInfo(reqMsg.arrayName))
        return POS_EVENT_ID::SUCCESS;
    else
        return POS_EVENT_ID::MFS_ARRAY_REMOVE_FAILED;
}

void
MetaFsSystemManager::_InitiateSystemRecovery(void)
{
    // metaFs recovery manager required
}
} // namespace pos

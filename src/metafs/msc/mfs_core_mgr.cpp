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

#include "mfs_core_mgr.h"

#include "mfs_mbr_mgr.h"

MetaFsCoreMgrClass mfsCoreMgr;
MetaFsMSCTopMgrClass& mscTopMgr = mfsCoreMgr;

MetaFsCoreMgrClass::MetaFsCoreMgrClass(void)
: isMfsUnmounted(false)
{
    _InitReqHandler();
}

MetaFsCoreMgrClass::~MetaFsCoreMgrClass(void)
{
}

MetaFsCoreMgrClass&
MetaFsCoreMgrClass::GetInstance(void)
{
    return mfsCoreMgr;
}

bool
MetaFsCoreMgrClass::Init(MetaStorageMediaInfoList& mediaInfoList)
{
    if (IsModuleReady())
    {
        MFS_TRACE_WARN((int)IBOF_EVENT_ID::MFS_WARNING_INIT_AGAIN,
            "You attempt to init. MetaFsMgr again in this power cycle and it is in normal state. Hence, ignore it..");
        return true;
    }
    mfsMBRMgr.Init(MetaStorageType::SSD, MetaFsMBRMgrClass::FILESYSTEM_MBR_BASE_LPN);

    for (auto& item : mediaInfoList)
    {
        mfsMBRMgr.RegisterVolumeGeometry(item);
    }

    mfsStateMgr.RequestSystemStateChange(MetaFsSystemState::PowerOn, MetaFsSystemState::Init);
    IBOF_EVENT_ID sc = mfsStateMgr.ExecuteStateTransition();
    if (sc != IBOF_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_MODULE_INIT_FAILED,
            "CoreMgr init failed.");
        return false;
    }
    SetModuleInit();

    if (isMfsUnmounted == true)
    {
        MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
            "Meta filesystem has been re-initialized without power off...");

        isMfsUnmounted = false;
    }

    return true;
}

void
MetaFsCoreMgrClass::_InitReqHandler(void)
{
    _RegisterReqHandler(MetaFsControlReqType::Mount, &MetaFsCoreMgrClass::_HandleMountReq);
    _RegisterReqHandler(MetaFsControlReqType::Unmount, &MetaFsCoreMgrClass::_HandleUnmountReq);
    _RegisterReqHandler(MetaFsControlReqType::FileSysCreate, &MetaFsCoreMgrClass::_HandleFileSysCreateReq);
}

void
MetaFsCoreMgrClass::_RegisterReqHandler(MetaFsControlReqType reqType, MetaFsControlReqHandlerPointer handler)
{
    reqHandler[(uint32_t)reqType] = handler;
}

bool
MetaFsCoreMgrClass::Bringup(void)
{
    mfsStateMgr.RequestSystemStateChange(MetaFsSystemState::Ready, MetaFsSystemState::Ready);
    IBOF_EVENT_ID sc = mfsStateMgr.ExecuteStateTransition();
    if (sc != IBOF_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_MODULE_BRINGUP_FAILED,
            "CoreMgr bringup failed.");
        return false;
    }

    SetModuleReady();

    return true;
}

IBOF_EVENT_ID
MetaFsCoreMgrClass::ProcessNewReq(MetaFsControlReqMsg& reqMsg)
{
    IBOF_EVENT_ID sc;
    sc = (this->*(reqHandler[(uint32_t)reqMsg.reqType]))(reqMsg);
    return sc;
}

bool
MetaFsCoreMgrClass::IsMounted(void)
{
    return (MetaFsSystemState::Active == mfsStateMgr.GetCurrSystemState());
}

IBOF_EVENT_ID
MetaFsCoreMgrClass::_HandleFileSysCreateReq(MetaFsControlReqMsg& reqMsg)
{
    if (mfsMBRMgr.IsValidMBRExist())
    {
        MFS_TRACE_WARN((int)IBOF_EVENT_ID::MFS_WARNING_INIT_AGAIN,
            "You attempt to create Meta filesystem again. Can't service this request. Ignore it.");
        return IBOF_EVENT_ID::SUCCESS;
    }

    mfsStateMgr.RequestSystemStateChange(MetaFsSystemState::Create, MetaFsSystemState::Create);
    IBOF_EVENT_ID sc = mfsStateMgr.ExecuteStateTransition();
    if (sc != IBOF_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED,
            "Meta filesystem creation request has been failed.");

        return sc;
    }

    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "Meta filesystem has been created...");

    return sc;
}

IBOF_EVENT_ID
MetaFsCoreMgrClass::_HandleMountReq(MetaFsControlReqMsg& reqMsg)
{
    if (MetaFsSystemState::Active != mfsStateMgr.GetCurrSystemState())
    {
        mfsStateMgr.RequestSystemStateChange(MetaFsSystemState::Open, MetaFsSystemState::Active);
        IBOF_EVENT_ID sc = mfsStateMgr.ExecuteStateTransition();
        if (sc != IBOF_EVENT_ID::SUCCESS)
        {
            MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_MEDIA_MOUNT_FAILED,
                "Meta filesystem mount request has been failed.");

            return sc;
        }
        MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
            "Meta filesystem has been mounted...");
    }
    else
    {
        MFS_TRACE_WARN((int)IBOF_EVENT_ID::MFS_SYSTEM_MOUNT_AGAIN,
            "Meta filesystem is already mounted. Ignore it");
    }

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaFsCoreMgrClass::_HandleUnmountReq(MetaFsControlReqMsg& reqMsg)
{
    IBOF_EVENT_ID sc = IBOF_EVENT_ID::SUCCESS;
    try
    {
        if (MetaFsSystemState::Shutdown != mfsStateMgr.GetCurrSystemState())
        {
            mfsStateMgr.RequestSystemStateChange(MetaFsSystemState::Quiesce, MetaFsSystemState::Shutdown);
            sc = mfsStateMgr.ExecuteStateTransition();
            if (sc != IBOF_EVENT_ID::SUCCESS)
            {
                // due to both array stop state and active files.
                MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_MEDIA_UNMOUNT_FAILED,
                    "Meta filesystem unmount request has been failed.");

                throw sc;
            }
            else
            {
                MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
                    "Meta filesystem has been unmounted...");

                throw IBOF_EVENT_ID::SUCCESS;
            }
        }
        else
        {
            MFS_TRACE_WARN((int)IBOF_EVENT_ID::MFS_SYSTEM_UNMOUNT_AGAIN,
                "Meta filesystem is already unmounted. Ignore it");

            throw IBOF_EVENT_ID::SUCCESS;
        }
    }
    catch (IBOF_EVENT_ID event)
    {
        if (sc != IBOF_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED_DUE_TO_ACTIVE_FILE)
        {
            SetModuleInit();
            isMfsUnmounted = true;
        }

        return event;
    }
}

void
MetaFsCoreMgrClass::_InitiateSystemRecovery(void)
{
    // metaFs recovery manager required
}

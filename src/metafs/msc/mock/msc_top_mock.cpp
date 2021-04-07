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

#include "msc_top_mock.h"

#include "mfs_mim_top.h"
#include "mfs_mvm_top.h"

MockMetaFsCoreMgrClass mockMetaFsCoreMgr;
MetaFsMSCTopMgrClass& mscTopMgr = mockMetaFsCoreMgr;

MockMetaFsCoreMgrClass::MockMetaFsCoreMgrClass(void)
{
}

MockMetaFsCoreMgrClass::~MockMetaFsCoreMgrClass(void)
{
}

MockMetaFsCoreMgrClass&
MockMetaFsCoreMgrClass::GetInstance(void)
{
    return mockMetaFsCoreMgr;
}

bool
MockMetaFsCoreMgrClass::Init(MetaStorageMediaInfoList& mediaInfoList)
{
    this->mediaInfoList = mediaInfoList;

    for (auto& item : mediaInfoList)
    {
        MetaVolumeType volumeType = MetaFsUtilLib::ConvertToVolumeType(item.media);
        MetaLpnType maxVolumeLpn = item.mediaCapacity / MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
        mvmTopMgr.Init(volumeType, maxVolumeLpn);
    }
    mimTopMgr.Init();

    SetModuleInit();

    return true;
}

bool
MockMetaFsCoreMgrClass::Bringup(void)
{
    mvmTopMgr.Bringup();
    mimTopMgr.Bringup();

    SetModuleReady();

    return true;
}

IBOF_EVENT_ID
MockMetaFsCoreMgrClass::ProcessNewReq(MetaFsControlReqMsg& reqMsg)
{
    switch (reqMsg.reqType)
    {
        case MetaFsControlReqType::FileSysCreate:
        {
            for (auto& item : mediaInfoList)
            {
                IBOF_EVENT_ID rc;
                rc = metaStorage->CreateMetaStore(item.media, item.mediaCapacity, true);
                if (rc != IBOF_EVENT_ID::SUCCESS)
                {
                    IBOF_TRACE_ERROR((int)rc, "Failed to mount meta storage subsystem");
                    return IBOF_EVENT_ID::MFS_MODULE_NOT_READY;
                }
            }
        }
        break;
        case MetaFsControlReqType::Mount:
        {
        }
        break;
        case MetaFsControlReqType::Unmount:
        {
        }
        break;
        default:
            assert(false);
    }
    return IBOF_EVENT_ID::SUCCESS;
}

bool
MockMetaFsCoreMgrClass::IsMounted(void)
{
    return true;
}

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
#include "meta_io_manager.h"

namespace pos
{
MockMetaFsCoreManager mockMetaFsCoreMgr;
MetaFsMSCTopManager& mscTopMgr = mockMetaFsCoreMgr;

MockMetaFsCoreManager::MockMetaFsCoreManager(void)
{
}

MockMetaFsCoreManager::~MockMetaFsCoreManager(void)
{
}

MockMetaFsCoreManager&
MockMetaFsCoreManager::GetInstance(void)
{
    return mockMetaFsCoreMgr;
}

bool
MockMetaFsCoreManager::Init(MetaStorageMediaInfoList& mediaInfoList)
{
    this->mediaInfoList = mediaInfoList;

    for (auto& item : mediaInfoList)
    {
        MetaVolumeType volumeType = MetaFileUtil::ConvertToVolumeType(item.media);
        MetaLpnType maxVolumeLpn = item.mediaCapacity / MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
        mvmTopMgr.Init(volumeType, maxVolumeLpn);
    }
    mimTopMgr.Init();

    SetModuleInit();

    return true;
}

bool
MockMetaFsCoreManager::Bringup(void)
{
    mvmTopMgr.Bringup();
    mimTopMgr.Bringup();

    SetModuleReady();

    return true;
}

POS_EVENT_ID
MockMetaFsCoreManager::ProcessNewReq(MetaFsControlReqMsg& reqMsg)
{
    switch (reqMsg.reqType)
    {
        case MetaFsControlReqType::CreateSystem:
        {
            for (auto& item : mediaInfoList)
            {
                POS_EVENT_ID rc;
                rc = metaStorage->CreateMetaStore(*reqMsg.arrayName, item.media, item.mediaCapacity, true);
                if (rc != POS_EVENT_ID::SUCCESS)
                {
                    POS_TRACE_ERROR((int)rc, "Failed to mount meta storage subsystem");
                    return POS_EVENT_ID::MFS_MODULE_NOT_READY;
                }
            }
        }
        break;
        case MetaFsControlReqType::MountSystem:
        {
        }
        break;
        case MetaFsControlReqType::UnmountSystem:
        {
        }
        break;
        default:
            assert(false);
    }
    return POS_EVENT_ID::SUCCESS;
}
} // namespace pos

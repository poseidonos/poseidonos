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

/* 
 * PoseidonOS - Meta File mgmt Layer
 * 
 * Meta File System Manager
*/

#include <string>
#include "metafs_system_manager.h"
#include "src/metafs/msc/metafs_mbr_mgr.h"
#include "meta_io_manager.h"

namespace pos
{
MetaFsSystemManager::MetaFsSystemManager(int arrayId,
        MetaStorageSubsystem* metaStorage, MetaFsMBRManager* mbrMgr)
: mbrMgr(mbrMgr),
  metaStorage(metaStorage)
{
    if (nullptr == this->mbrMgr)
    {
        this->mbrMgr = new MetaFsMBRManager(arrayId);
    }
    _InitReqHandler();
}

MetaFsSystemManager::~MetaFsSystemManager(void)
{
    if (nullptr != mbrMgr)
        delete mbrMgr;
}

POS_EVENT_ID
MetaFsSystemManager::CheckReqSanity(MetaFsRequestBase& reqMsg)
{
    POS_EVENT_ID sc = EID(SUCCESS);
    MetaFsControlReqMsg* msg = static_cast<MetaFsControlReqMsg*>(&reqMsg);

    if (false == msg->IsValid())
    {
        MFS_TRACE_ERROR(EID(MFS_INVALID_PARAMETER),
            "Given request is incorrect. Please check parameters.");
        return EID(MFS_INVALID_PARAMETER);
    }

    return sc;
}

bool
MetaFsSystemManager::Init(MetaStorageInfoList& mediaInfoList)
{
    mbrMgr->Init(MetaStorageType::SSD, MetaFsMBRManager::FILESYSTEM_MBR_BASE_LPN);
    mbrMgr->SetMss(metaStorage);

    for (auto& item : mediaInfoList)
    {
        mbrMgr->RegisterVolumeGeometry(item);
    }

    return true;
}

void
MetaFsSystemManager::_InitReqHandler(void)
{
    _RegisterReqHandler(MetaFsControlReqType::InitializeSystem, &MetaFsSystemManager::_HandleInitializeRequest);
    _RegisterReqHandler(MetaFsControlReqType::CloseSystem,      &MetaFsSystemManager::_HandleCloseRequest);
}

void
MetaFsSystemManager::_RegisterReqHandler(MetaFsControlReqType reqType, MetaFsControlReqHandlerPointer handler)
{
    reqHandler[(uint32_t)reqType] = handler;
}

POS_EVENT_ID
MetaFsSystemManager::ProcessNewReq(MetaFsRequestBase& reqMsg)
{
    POS_EVENT_ID sc;
    MetaFsControlReqMsg* msg = static_cast<MetaFsControlReqMsg*>(&reqMsg);
    sc = (this->*(reqHandler[(uint32_t)msg->reqType]))(*msg);
    return sc;
}

uint64_t
MetaFsSystemManager::GetEpochSignature(void)
{
    return mbrMgr->GetEpochSignature();
}

MetaFsStorageIoInfoList&
MetaFsSystemManager::GetAllStoragePartitionInfo(void)
{
    return mbrMgr->GetAllStoragePartitionInfo();
}

MetaLpnType
MetaFsSystemManager::GetRegionSizeInLpn(void)
{
    return mbrMgr->GetRegionSizeInLpn();
}

POS_EVENT_ID
MetaFsSystemManager::LoadMbr(bool& isNPOR)
{
    if (true == mbrMgr->LoadMBR())
    {
        if (false == mbrMgr->IsValidMBRExist())
        {
            MFS_TRACE_ERROR(EID(MFS_INVALID_MBR),
                "Filesystem MBR has been corrupted or Filesystem cannot be found.");

            return EID(MFS_INVALID_MBR);
        }

        isNPOR = mbrMgr->GetPowerStatus();
        if (true == isNPOR)
        {
            MFS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
                "This open is NPOR case!!!");
        }
    }
    else
    {
        MFS_TRACE_ERROR(EID(MFS_META_LOAD_FAILED),
            "Error occurred while loading filesystem MBR");

        return EID(MFS_META_LOAD_FAILED);
    }

    return EID(SUCCESS);
}

bool
MetaFsSystemManager::IsMbrClean(void)
{
    if (0 == mbrMgr->GetEpochSignature())
        return true;

    return false;
}

bool
MetaFsSystemManager::CreateMbr(void)
{
    return mbrMgr->CreateMBR();
}

POS_EVENT_ID
MetaFsSystemManager::_HandleInitializeRequest(MetaFsControlReqMsg& reqMsg)
{
    if (true == Init(*reqMsg.mediaList))
        return EID(SUCCESS);

    return EID(MFS_ERROR_MOUNTED);
}

POS_EVENT_ID
MetaFsSystemManager::_HandleCloseRequest(MetaFsControlReqMsg& reqMsg)
{
    POS_EVENT_ID rc = EID(SUCCESS);

    do
    {
        mbrMgr->SetPowerStatus(true /*NPOR status*/);

        if (true != mbrMgr->SaveContent())
        {
            rc = EID(MFS_META_SAVE_FAILED);
            break;
        }

        mbrMgr->InvalidMBR();

        rc = metaStorage->Close();
        if (EID(SUCCESS) != rc)
        {
            rc = EID(MFS_META_STORAGE_CLOSE_FAILED);
            break;
        }
    } while (0);

    return rc;
}
} // namespace pos

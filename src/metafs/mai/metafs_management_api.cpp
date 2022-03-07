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

#include <string>
#include "metafs_management_api.h"
#include "msc_req.h"

namespace pos
{
MetaFsManagementApi::MetaFsManagementApi(void)
{
}

MetaFsManagementApi::MetaFsManagementApi(int arrayId, MetaStorageSubsystem* storage,
                                MetaFsSystemManager* sysMgr)
{
    this->arrayId = arrayId;
    this->sysMgr = (nullptr == sysMgr) ? new MetaFsSystemManager(arrayId, storage) : sysMgr;
}

MetaFsManagementApi::~MetaFsManagementApi(void)
{
    delete sysMgr;
}

POS_EVENT_ID
MetaFsManagementApi::InitializeSystem(int arrayId,
                                MetaStorageInfoList* mediaInfoList)
{
    POS_EVENT_ID rc;
    MetaFsControlReqMsg reqMsg;

    reqMsg.reqType = MetaFsControlReqType::InitializeSystem;
    reqMsg.arrayId = arrayId;
    reqMsg.mediaList = mediaInfoList;

    rc = sysMgr->HandleNewRequest(reqMsg); // MetaFsSystemManager::_HandleInitializeRequest()

    return rc;
}

POS_EVENT_ID
MetaFsManagementApi::CloseSystem(int arrayId)
{
    POS_EVENT_ID rc;
    MetaFsControlReqMsg reqMsg;

    reqMsg.reqType = MetaFsControlReqType::CloseSystem;
    reqMsg.arrayId = arrayId;

    rc = sysMgr->HandleNewRequest(reqMsg); // MetaFsSystemManager::_HandleCloseRequest()

    return rc;
}

uint64_t
MetaFsManagementApi::GetEpochSignature(void)
{
    return sysMgr->GetEpochSignature();
}

MetaFsStorageIoInfoList&
MetaFsManagementApi::GetAllStoragePartitionInfo(void)
{
    return sysMgr->GetAllStoragePartitionInfo();
}

MetaLpnType
MetaFsManagementApi::GetRegionSizeInLpn(void)
{
    return sysMgr->GetRegionSizeInLpn();
}

POS_EVENT_ID
MetaFsManagementApi::LoadMbr(bool& isNPOR)
{
    return sysMgr->LoadMbr(isNPOR);
}

bool
MetaFsManagementApi::CreateMbr(void)
{
    return sysMgr->CreateMbr();
}

bool
MetaFsManagementApi::IsMbrClean(void)
{
    return sysMgr->IsMbrClean();
}

void
MetaFsManagementApi::SetStatus(bool isNormal)
{
    this->isNormal = isNormal;
}
} // namespace pos

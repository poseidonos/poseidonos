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

#pragma once

#include "metafs_manager_base.h"
#include "metafs_return_code.h"
#include "msc_req.h"
#include "src/metafs/msc/metafs_mbr_mgr.h"
#include "src/metafs/include/meta_storage_info.h"

#include <string>

namespace pos
{
class MetaFsSystemManager;
using MetaFsControlReqHandlerPointer = POS_EVENT_ID (MetaFsSystemManager::*)(MetaFsControlReqMsg&);

class MetaFsSystemManager : public MetaFsManagerBase
{
public:
    explicit MetaFsSystemManager(int arrayId, MetaStorageSubsystem* metaStorage,
        MetaFsMBRManager* mbrMgr = nullptr);
    virtual ~MetaFsSystemManager(void);

    virtual bool Init(MetaStorageInfoList& mediaInfoList);
    virtual POS_EVENT_ID CheckReqSanity(MetaFsRequestBase& reqMsg);
    virtual POS_EVENT_ID ProcessNewReq(MetaFsRequestBase& reqMsg);

    virtual uint64_t GetEpochSignature(void);
    virtual MetaFsStorageIoInfoList& GetAllStoragePartitionInfo(void);
    virtual MetaLpnType GetRegionSizeInLpn(void);
    virtual POS_EVENT_ID LoadMbr(bool& isNPOR);
    virtual bool CreateMbr(void);
    virtual bool IsMbrClean(void);

private:
    void _InitReqHandler(void);
    void _RegisterReqHandler(MetaFsControlReqType reqType, MetaFsControlReqHandlerPointer handler);

    POS_EVENT_ID _HandleInitializeRequest(MetaFsControlReqMsg& reqMsg);
    POS_EVENT_ID _HandleCloseRequest(MetaFsControlReqMsg& reqMsg);

    MetaFsControlReqHandlerPointer reqHandler[(uint32_t)MetaFsControlReqType::Max];

    MetaFsMBRManager* mbrMgr = nullptr;
    MetaStorageSubsystem* metaStorage = nullptr;
};
} // namespace pos

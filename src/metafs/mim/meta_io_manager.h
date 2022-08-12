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
 * PoseidonOS - Meta Filesystem Layer
 * 
 * Meta File I/O Manager
*/
#pragma once

#include <string>
#include <unordered_map>

#include "meta_io_manager.h"
#include "meta_volume_manager.h"
#include "metafs_io_request.h"
#include "metafs_manager_base.h"

namespace pos
{
class MetaIoManager;
class MetaStorageSubsystem;
class MetaFsIoScheduler;

using MetaIoReqHandler = POS_EVENT_ID (MetaIoManager::*)(MetaFsIoRequest& reqMsg);
using SchedulerMap = std::unordered_map<uint32_t, MetaFsIoScheduler*>;

class MetaIoManager : public MetaFsManagerBase
{
public:
    // only for test
    MetaIoManager(const bool supportNumaDedicated, MetaStorageSubsystem* storage = nullptr);
    MetaIoManager(const bool supportNumaDedicated, const SchedulerMap& ioScheduler, MetaStorageSubsystem* storage);
    virtual ~MetaIoManager(void);

    bool IsSuccess(POS_EVENT_ID rc);

    virtual void Init(void);
    virtual POS_EVENT_ID CheckReqSanity(MetaFsRequestBase& reqMsg);
    virtual POS_EVENT_ID ProcessNewReq(MetaFsRequestBase& reqMsg);
    void Finalize(void);

    bool AddArrayInfo(const int arrayId, const MaxMetaLpnMapPerMetaStorage& map);
    bool RemoveArrayInfo(const int arrayId);

private:
    void _InitReqHandler(void);

    POS_EVENT_ID _ProcessNewIoReq(MetaFsIoRequest& reqMsg);
    void _SetByteRangeForFullFileIo(MetaFsIoRequest& reqMsg);
    void _SetTargetMediaType(MetaFsIoRequest& reqMsg);
    void _WaitForDone(MetaFsIoRequest& reqMsg);
    void _IssueToScheduler(MetaFsIoRequest* reqMsg);

    static const uint32_t NUM_IO_TYPE = static_cast<uint32_t>(MetaIoRequestType::Max);
    const bool IS_SINGLE_SCHEDULER;
    const bool SUPPORT_NUMA_DEDICATED_SCHEDULING;
    MetaIoReqHandler reqHandler[NUM_IO_TYPE];
    SchedulerMap ioScheduler;
    uint32_t totalMetaIoCoreCnt;
    uint32_t mioHandlerCount;
    bool finalized;
    MetaStorageSubsystem* metaStorage;
};
} // namespace pos

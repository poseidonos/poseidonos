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

#pragma once

#include "mfs_mutex.h"
#include "mfs_ret_code.h"
#include "src/logger/logger.h"

template<class MetaFsBTopMgrT, class MetaReqMsgT, class ReqRetType = IBOF_EVENT_ID>
class MetaFsFBAdapterClass
{
public:
    MetaFsFBAdapterClass(void);

    ReqRetType HandleNewReq(MetaReqMsgT& reqMsg);

    bool IsMounted(void);

private:
    std::mutex cmdSerializer;
};

template<class MetaFsBTopMgrT, class MetaReqMsgT, class ReqRetType>
MetaFsFBAdapterClass<MetaFsBTopMgrT, MetaReqMsgT, ReqRetType>::MetaFsFBAdapterClass(void)
{
}

template<class MetaFsBTopMgrT, class MetaReqMsgT, class ReqRetType>
ReqRetType
MetaFsFBAdapterClass<MetaFsBTopMgrT, MetaReqMsgT, ReqRetType>::HandleNewReq(MetaReqMsgT& reqMsg)
{
    // FIXME: as of now, for any given request, we just serialize it to avoid any data and operation consisteny issue
    MetaFsBTopMgrT& topMgr = MetaFsBTopMgrT::GetInstance();

    bool ret = topMgr.CheckModuleReadiness();
    if (false == ret)
    {
        IBOF_TRACE_ERROR((int)ReqRetType::MFS_MODULE_NOT_READY,
            "Module is not ready. (Module={}, current state={})",
            topMgr.GetModuleName(), (int)topMgr.GetModuleState());
        IBOF_TRACE_ERROR((int)ReqRetType::MFS_MODULE_NOT_READY,
            "The request couldn't get executed. Please send the request after it gets ready to service");
        return ReqRetType::MFS_MODULE_NOT_READY;
    }

    ReqRetType rc = topMgr.CheckReqSanity(reqMsg);
    if (rc != ReqRetType::SUCCESS)
    {
        IBOF_TRACE_ERROR((int)rc, "Command sanity error detected!");
        return rc;
    }
    rc = topMgr.ProcessNewReq(reqMsg);

    return rc;
}

template<class MetaFsBTopMgrT, class MetaReqMsgT, class ReqRetType>
bool
MetaFsFBAdapterClass<MetaFsBTopMgrT, MetaReqMsgT, ReqRetType>::IsMounted(void)
{
    MetaFsBTopMgrT& topMgr = MetaFsBTopMgrT::GetInstance();

    return topMgr.IsMounted();
}

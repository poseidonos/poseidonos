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

#include "mfs_mim_top.h"

#include "mfs_log.h"
#include "mfs_mvm_top.h"
#include "mvm_req.h"
#include "src/logger/logger.h"

MetaFsMIMTopMgrClass::MetaFsMIMTopMgrClass(void)
{
}

const char*
MetaFsMIMTopMgrClass::GetModuleName(void)
{
    return "MIM topMgr";
}

void
MetaFsMIMTopMgrClass::SetMDpageEpochSignature(uint64_t mbrEpochSignature)
{
}

IBOF_EVENT_ID
MetaFsMIMTopMgrClass::CheckReqSanity(MetaFsIoReqMsg& reqMsg)
{
    IBOF_EVENT_ID rc = IBOF_EVENT_ID::SUCCESS;

    if (false == reqMsg.IsValid())
    {
        return IBOF_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    IBOF_EVENT_ID mgmtSC;
    mgmtSC = mvmTopMgr.CheckFileAccessible(reqMsg.fd);
    if (IBOF_EVENT_ID::SUCCESS != mgmtSC)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_FILE_NOT_FOUND,
            "File not found...(given fd={})", reqMsg.fd);
        return IBOF_EVENT_ID::MFS_FILE_NOT_FOUND;
    }

    switch (reqMsg.reqType)
    {
        case MetaIoReqTypeEnum::Read: // go thru
        case MetaIoReqTypeEnum::Write:
        {
            if (MetaIoModeEnum::Async == reqMsg.ioMode)
            {
                rc = _CheckAIOReqSanity(reqMsg);
                if (!IsSuccess(rc))
                {
                    return rc;
                }
            }
        }
        break;
        default:
        {
            MFS_TRACE_CRITICAL((int)IBOF_EVENT_ID::MFS_INVALID_PARAMETER,
                "MetaFsMIMTopMgrClass::CheckReqSanity - Invalid OPcode");
            assert(false);
        }
    }
    return rc;
}

IBOF_EVENT_ID
MetaFsMIMTopMgrClass::_CheckAIOReqSanity(MetaFsIoReqMsg& reqMsg)
{
    return IBOF_EVENT_ID::SUCCESS;
}

bool
MetaFsMIMTopMgrClass::_IsSiblingModuleReady(void)
{
    // explicitly check. better to know which friends are associated to MIM
    if (false == mvmTopMgr.IsModuleReady())
    {
        return false;
    }
    // any other friends?

    return true;
}

bool
MetaFsMIMTopMgrClass::IsSuccess(IBOF_EVENT_ID rc)
{
    return rc == IBOF_EVENT_ID::SUCCESS;
}

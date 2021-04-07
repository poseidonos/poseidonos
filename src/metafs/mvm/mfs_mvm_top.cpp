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

#include "mfs_mvm_top.h"

#include "mf_inode_mgr.h"
#include "src/logger/logger.h"

MetaFsMVMTopMgrClass::MetaFsMVMTopMgrClass(void)
{
}

const char*
MetaFsMVMTopMgrClass::GetModuleName(void)
{
    return "MVM topMgr";
}

IBOF_EVENT_ID
MetaFsMVMTopMgrClass::CheckReqSanity(MetaFsMoMReqMsg& reqMsg)
{
    IBOF_EVENT_ID sc = _CheckSanityBasic(reqMsg);
    if (sc != IBOF_EVENT_ID::SUCCESS)
    {
        return sc;
    }

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaFsMVMTopMgrClass::_CheckSanityBasic(MetaFsMoMReqMsg& reqMsg)
{
    if (false == reqMsg.IsValid())
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_INVALID_PARAMETER,
            "Given request is incorrect. Please check parameters.");
        return IBOF_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaFsMVMTopMgrClass::CheckFileAccessible(FileFDType fd)
{
    IBOF_EVENT_ID rc;

    MetaFsMoMReqMsg req;
    req.reqType = MetaFsMoMReqType::CheckFileAccessible;
    req.fd = fd;
    rc = mvmTopMgr.ProcessNewReq(req);

    return rc;
}

IBOF_EVENT_ID
MetaFsMVMTopMgrClass::GetFileSize(FileFDType fd, FileSizeType& outFileByteSize)
{
    MetaFsMoMReqMsg req;
    req.reqType = MetaFsMoMReqType::GetFileSize;
    req.fd = fd;
    IBOF_EVENT_ID ret = mvmTopMgr.ProcessNewReq(req);
    if (ret == IBOF_EVENT_ID::SUCCESS)
    {
        outFileByteSize = req.completionData.fileSize;
    }
    else
    {
        outFileByteSize = 0;
    }

    return ret;
}

IBOF_EVENT_ID
MetaFsMVMTopMgrClass::GetDataChunkSize(FileFDType fd, FileSizeType& outDataChunkSize)
{
    MetaFsMoMReqMsg req;
    req.reqType = MetaFsMoMReqType::GetDataChunkSize;
    req.fd = fd;
    IBOF_EVENT_ID ret = mvmTopMgr.ProcessNewReq(req);
    if (ret == IBOF_EVENT_ID::SUCCESS)
    {
        outDataChunkSize = req.completionData.dataChunkSize;
    }
    return ret;
}

IBOF_EVENT_ID
MetaFsMVMTopMgrClass::GetTargetMediaType(FileFDType fd, MetaStorageType& outTargetMediaType)
{
    MetaFsMoMReqMsg req;
    req.reqType = MetaFsMoMReqType::GetTargetMediaType;
    req.fd = fd;
    IBOF_EVENT_ID ret = mvmTopMgr.ProcessNewReq(req);
    if (ret == IBOF_EVENT_ID::SUCCESS)
    {
        outTargetMediaType = req.completionData.targetMediaType;
    }
    return ret;
}

IBOF_EVENT_ID
MetaFsMVMTopMgrClass::GetFileBaseLpn(FileFDType fd, MetaLpnType& outFileBaseLpn)
{
    MetaFsMoMReqMsg req;
    req.reqType = MetaFsMoMReqType::GetFileBaseLpn;
    req.fd = fd;
    IBOF_EVENT_ID ret = mvmTopMgr.ProcessNewReq(req);
    if (ret == IBOF_EVENT_ID::SUCCESS)
    {
        outFileBaseLpn = req.completionData.fileBaseLpn;
    }
    return ret;
}

bool
MetaFsMVMTopMgrClass::_IsSiblingModuleReady(void)
{
    return true;
}

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

#include "mfs_util_api.h"

// return file size of corresponding file mapped to 'fd'.
// in fail case, return 0
size_t
MetaFsUtilApiWrapperClass::GetFileSize(int fd)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsMoMReqMsg reqMsg;

    reqMsg.reqType = MetaFsMoMReqType::GetFileSize;
    reqMsg.fd = fd;
    rc = mvm.HandleNewReq(reqMsg); // MetaVolMgrClass::_HandleGetFileSizeReq()

    if (rc.IsSuccess())
    {
        return reqMsg.completionData.fileSize;
    }
    else
    {
        return 0;
    }
}

// return data chunk size of corresponding file mapped to 'fd'.
// in fail case, return 0
size_t
MetaFsUtilApiWrapperClass::GetAlignedFileIOSize(int fd)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsMoMReqMsg reqMsg;

    reqMsg.reqType = MetaFsMoMReqType::GetDataChunkSize;
    reqMsg.fd = fd;
    rc = mvm.HandleNewReq(reqMsg); // MetaVolMgrClass::_HandleGetDataChunkSizeReq()

    if (rc.IsSuccess())
    {
        return reqMsg.completionData.dataChunkSize;
    }
    else
    {
        return 0;
    }
}

size_t
MetaFsUtilApiWrapperClass::GetAtomicWriteUnitSize(int fd)
{
    return GetAlignedFileIOSize(fd) * MetaFsConfig::MAX_ATOMIC_WRITE_PAGE_CNT;
}

// return data chunk size according to the data integrity level
size_t
MetaFsUtilApiWrapperClass::EstimateAlignedFileIOSize(MetaFilePropertySet& prop)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsMoMReqMsg reqMsg;

    reqMsg.reqType = MetaFsMoMReqType::EstimateDataChunkSize;
    reqMsg.fileProperty = prop;
    rc = mvm.HandleNewReq(reqMsg); // MetaVolMgrClass::_HandleEstimateDataChunkSizeReq()

    if (rc.IsSuccess())
    {
        return reqMsg.completionData.dataChunkSize;
    }
    else
    {
        return 0;
    }
}

size_t
MetaFsUtilApiWrapperClass::GetTheBiggestExtentSize(MetaFilePropertySet& prop)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsMoMReqMsg reqMsg;

    reqMsg.reqType = MetaFsMoMReqType::GetTheBiggestExtentSize;
    reqMsg.fileByteSize = 0;
    reqMsg.fileProperty = prop;
    rc = mvm.HandleNewReq(reqMsg); // MetaVolMgrClass::_HandleGetFreeFileRegionSizeReq()

    if (rc.IsSuccess())
    {
        return reqMsg.completionData.fileSize;
    }
    else
    {
        return 0;
    }
}

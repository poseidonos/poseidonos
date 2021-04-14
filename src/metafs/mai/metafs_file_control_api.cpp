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

#include "metafs_file_control_api.h"

namespace pos
{
MetaFsReturnCode<POS_EVENT_ID>
MetaFsFileControlApi::CreateVolume(std::string& fileName, std::string& arrayName, uint64_t fileByteSize, MetaFilePropertySet prop)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsFileControlRequest reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaFsFileControlType::FileCreate;
    reqMsg.fileName = &fileName;
    reqMsg.arrayName = &arrayName;
    reqMsg.fileByteSize = fileByteSize;
    reqMsg.fileProperty = prop;

    rc = volMgr.HandleNewRequest(reqMsg); // validity check & MetaVolumeManager::HandleCreateFileReq()

    return rc;
}

MetaFsReturnCode<POS_EVENT_ID>
MetaFsFileControlApi::Delete(std::string& fileName, std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsFileControlRequest reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaFsFileControlType::FileDelete;
    reqMsg.fileName = &fileName;
    reqMsg.arrayName = &arrayName;
    rc = volMgr.HandleNewRequest(reqMsg);

    return rc;
}

MetaFsReturnCode<POS_EVENT_ID>
MetaFsFileControlApi::Open(std::string& fileName, std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsFileControlRequest reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaFsFileControlType::FileOpen;
    reqMsg.fileName = &fileName;
    reqMsg.arrayName = &arrayName;
    rc = volMgr.HandleNewRequest(reqMsg); // validity check & MetaVolumeManager::HandleOpenFileReq()

    rc.returnData = reqMsg.completionData.openfd;
    return rc;
}

MetaFsReturnCode<POS_EVENT_ID>
MetaFsFileControlApi::Close(uint32_t fd, std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsFileControlRequest reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaFsFileControlType::FileClose;
    reqMsg.fd = fd;
    reqMsg.arrayName = &arrayName;
    rc = volMgr.HandleNewRequest(reqMsg); // validity check &  MetaVolumeManager::HandleCloseFileReq()

    return rc;
}

MetaFsReturnCode<POS_EVENT_ID>
MetaFsFileControlApi::CheckFileExist(std::string& fileName, std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsFileControlRequest reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaFsFileControlType::CheckFileExist;
    reqMsg.fileName = &fileName;
    reqMsg.arrayName = &arrayName;
    rc = volMgr.HandleNewRequest(reqMsg);

    return rc;
}

// return file size of corresponding file mapped to 'fd'.
// in fail case, return 0
size_t
MetaFsFileControlApi::GetFileSize(int fd, std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetFileSize;
    reqMsg.fd = fd;
    reqMsg.arrayName = &arrayName;
    rc = volMgr.HandleNewRequest(reqMsg); // MetaVolumeManager::HandleGetFileSizeReq()

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
MetaFsFileControlApi::GetAlignedFileIOSize(int fd, std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetDataChunkSize;
    reqMsg.fd = fd;
    reqMsg.arrayName = &arrayName;
    rc = volMgr.HandleNewRequest(reqMsg); // MetaVolumeManager::HandleGetDataChunkSizeReq()

    if (rc.IsSuccess())
    {
        return reqMsg.completionData.dataChunkSize;
    }
    else
    {
        return 0;
    }
}

// return data chunk size according to the data integrity level
size_t
MetaFsFileControlApi::EstimateAlignedFileIOSize(MetaFilePropertySet& prop, std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::EstimateDataChunkSize;
    reqMsg.fileProperty = prop;
    reqMsg.arrayName = &arrayName;
    rc = volMgr.HandleNewRequest(reqMsg); // MetaVolumeManager::HandleEstimateDataChunkSizeReq()

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
MetaFsFileControlApi::GetTheBiggestExtentSize(MetaFilePropertySet& prop, std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetTheBiggestExtentSize;
    reqMsg.fileByteSize = 0;
    reqMsg.fileProperty = prop;
    reqMsg.arrayName = &arrayName;
    rc = volMgr.HandleNewRequest(reqMsg); // MetaVolumeManager::HandleGetFreeFileRegionSizeReq()

    if (rc.IsSuccess())
    {
        return reqMsg.completionData.fileSize;
    }
    else
    {
        return 0;
    }
}
} // namespace pos

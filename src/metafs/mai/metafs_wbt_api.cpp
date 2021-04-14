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

#include "mk/ibof_config.h"

#include "metafs_wbt_api.h"

namespace pos
{
// WBT: Load file inode lists for each volume type
MetaFsReturnCode<MetaFsStatusCodeWBTSpcf, std::vector<MetaFileInfoDumpCxt>>
MetaFsWBTApi::GetMetaFileList(std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rcMgmt;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetMetaFileInfoList;
    reqMsg.arrayName = &arrayName;

    rcMgmt = mvm.HandleNewRequest(reqMsg); // MetaVolumeManager::_HandleGetMetaFileInodeListReq()

    MetaFsReturnCode<MetaFsStatusCodeWBTSpcf, std::vector<MetaFileInfoDumpCxt>> rc;
    if (rcMgmt.IsSuccess())
    {
        std::vector<MetaFileInfoDumpCxt>* fileInfoListPointer = reqMsg.completionData.fileInfoListPointer;

        if (fileInfoListPointer == nullptr)
        {
            rc.sc = MetaFsStatusCodeWBTSpcf::Fail;
            return rc;
        }
        for (unsigned int i = 0; i < (*fileInfoListPointer).size(); i++)
        {
            rc.returnData.push_back((*fileInfoListPointer)[i]);
        }
        delete fileInfoListPointer;

        rc.sc = MetaFsStatusCodeWBTSpcf::Success;
    }
    else
    {
        rc.sc = MetaFsStatusCodeWBTSpcf::Fail;
    }

    return rc;
}

// WBT : Get max file size limit (max volume LPN * 95%)
MetaFsReturnCode<MetaFsStatusCodeWBTSpcf, FileSizeType>
MetaFsWBTApi::GetMaxFileSizeLimit(void)
{
    MetaFsReturnCode<POS_EVENT_ID> rcMgmt;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetMaxFileSizeLimit;

    rcMgmt = mvm.HandleNewRequest(reqMsg); // MetaVolumeManager::_HandleGetMaxFileSizeLimitReq()

    MetaFsReturnCode<MetaFsStatusCodeWBTSpcf, FileSizeType> rc;
    rc.returnData = 0;
    if (rcMgmt.IsSuccess())
    {
        rc.returnData = reqMsg.completionData.maxFileSizeByteLimit;
        rc.sc = MetaFsStatusCodeWBTSpcf::Success;
    }
    else
    {
        rc.sc = MetaFsStatusCodeWBTSpcf::Fail;
    }

    return rc;
}

// WBT : Get file inode info. for the given meta file.
MetaFsReturnCode<MetaFsStatusCodeWBTSpcf, MetaFileInodeDumpCxt>
MetaFsWBTApi::GetMetaFileInode(std::string& fileName)
{
    MetaFsReturnCode<POS_EVENT_ID> rcMgmt;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetFileInode;
    reqMsg.fileName = &fileName;

    rcMgmt = mvm.HandleNewRequest(reqMsg); // MetaVolumeManager::_HandleGetFileInodeReq()

    MetaFsReturnCode<MetaFsStatusCodeWBTSpcf, MetaFileInodeDumpCxt> rc;
    if (rcMgmt.IsSuccess())
    {
        MetaFileInodeInfo* fileInodePointer = reqMsg.completionData.inodeInfoPointer;

        rc.returnData.inodeInfo = *fileInodePointer;
        rc.sc = MetaFsStatusCodeWBTSpcf::Success;
    }
    else
    {
        rc.sc = MetaFsStatusCodeWBTSpcf::Fail;
    }

    return rc;
}
} // namespace pos

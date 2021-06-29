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

/* 
 * PoseidonOS - Meta Filesystem Layer
 * 
 * Meta Volume Manager
*/
#pragma once

#include <string>
#include <utility>
#include "metafs_common.h"
#include "meta_volume.h"
#include "meta_volume_context.h"

namespace pos
{
class MetaVolumeHandler
{
public:
    MetaVolumeHandler(void);
    ~MetaVolumeHandler(void);

    void InitHandler(MetaVolumeContext* volContext);

    POS_EVENT_ID HandleOpenFileReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleCloseFileReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleCreateFileReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleDeleteFileReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleGetDataChunkSizeReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleCheckFileAccessibleReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleGetFileSizeReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleGetTargetMediaTypeReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleGetFileBaseLpnReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleGetFreeFileRegionSizeReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleCheckFileExist(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleCreateArrayReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleDeleteArrayReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleGetMaxMetaLpnReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);

    // WBT
    POS_EVENT_ID HandleGetMetaFileInodeListReq(MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleGetMaxFileSizeLimitReq(MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID HandleGetFileInodeReq(MetaFsFileControlRequest& reqMsg);

    POS_EVENT_ID HandleEstimateDataChunkSizeReq(MetaFsFileControlRequest& reqMsg);

    POS_EVENT_ID CheckFileAccessible(FileDescriptorType fd);
    POS_EVENT_ID GetFileSize(FileDescriptorType fd, FileSizeType& outFileByteSize);
    POS_EVENT_ID GetDataChunkSize(FileDescriptorType fd, FileSizeType& outDataChunkSize);
    POS_EVENT_ID GetTargetMediaType(FileDescriptorType fd, MetaStorageType& outTargetMediaType);
    POS_EVENT_ID GetFileBaseLpn(FileDescriptorType fd, MetaLpnType& outFileBaseLpn);

private:
    bool _CheckFileCreateReqSanity(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);

    MetaVolumeContext* volContext;
};
} // namespace pos

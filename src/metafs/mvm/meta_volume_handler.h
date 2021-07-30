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
#include "src/metafs/mvm/volume/meta_volume.h"
#include "meta_volume_container.h"

namespace pos
{
class MetaVolumeHandler
{
public:
    MetaVolumeHandler(void);
    virtual ~MetaVolumeHandler(void);

    virtual void InitHandler(MetaVolumeContainer* volContainer);

    virtual POS_EVENT_ID HandleOpenFileReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleCloseFileReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleCreateFileReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleDeleteFileReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetDataChunkSizeReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleCheckFileAccessibleReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetFileSizeReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetTargetMediaTypeReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetFileBaseLpnReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetFreeFileRegionSizeReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleCheckFileExist(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleCreateArrayReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleDeleteArrayReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetMaxMetaLpnReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);

    // WBT
    virtual POS_EVENT_ID HandleGetMetaFileInodeListReq(MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetFileInodeReq(MetaFsFileControlRequest& reqMsg);

    virtual POS_EVENT_ID HandleEstimateDataChunkSizeReq(MetaFsFileControlRequest& reqMsg);

private:
    bool _CheckFileCreateReqSanity(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);

    MetaVolumeContainer* volContainer;
};
} // namespace pos

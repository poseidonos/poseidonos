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
 * Meta Volume Manager
*/
#pragma once

#include <string>

#include "meta_volume_container.h"
#include "src/metafs/common/metafs_common.h"
#include "src/metafs/mvm/volume/meta_volume.h"
#include "src/telemetry/telemetry_client/pos_metric.h"

namespace pos
{
class TelemetryPublisher;

class MetaVolumeHandler
{
public:
    MetaVolumeHandler(void) = delete;
    explicit MetaVolumeHandler(MetaVolumeContainer* volContainer, TelemetryPublisher* tp = nullptr);
    virtual ~MetaVolumeHandler(void);

    virtual POS_EVENT_ID HandleOpenFileReq(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleCloseFileReq(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleCreateFileReq(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleDeleteFileReq(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetDataChunkSizeReq(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleCheckFileAccessibleReq(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetFileSizeReq(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetTargetMediaTypeReq(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetFileBaseLpnReq(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetFreeFileRegionSizeReq(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleCheckFileExist(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleCreateArrayReq(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleDeleteArrayReq(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetMaxMetaLpnReq(const MetaVolumeType volType,
        MetaFsFileControlRequest& reqMsg);

    // WBT
    virtual POS_EVENT_ID HandleGetMetaFileInodeListReq(MetaFsFileControlRequest& reqMsg);
    virtual POS_EVENT_ID HandleGetFileInodeReq(MetaFsFileControlRequest& reqMsg);

    virtual POS_EVENT_ID HandleEstimateDataChunkSizeReq(MetaFsFileControlRequest& reqMsg);

private:
    POS_EVENT_ID _CheckFileCreateReqSanity(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    void _PublishMetricConditionally(const std::string& name, const POSMetricTypes metricType,
        const int arrayId, const MetaVolumeType volType, const MetaFileType fileType,
        const bool requestResult);

    MetaVolumeContainer* volContainer;
    TelemetryPublisher* tp;
};
} // namespace pos

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
#include "src/metafs/storage/mss.h"
#include "metafs_manager_base.h"
#include "file_descriptor_manager.h"
#include "meta_file_manager.h"
#include "meta_volume.h"
#include "volume_catalog_manager.h"
#include "meta_volume_handler.h"
#include "meta_volume_context.h"
#include "mf_inode_mgr.h"
#include "metafs_common.h"
#include "mk/ibof_config.h"
#include "metafs_control_request.h"
#include "nvram_meta_volume.h"
#include "ssd_meta_volume.h"

namespace pos
{
using MetaVolSpcfReqHandler = POS_EVENT_ID (MetaVolumeHandler::*)(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
using GlobalMetaReqHandler = POS_EVENT_ID (MetaVolumeHandler::*)(MetaFsFileControlRequest& reqMsg);

class MetaVolumeManager : public MetaFsManagerBase
{
public:
    MetaVolumeManager(void);
    virtual ~MetaVolumeManager(void);

    const char* GetModuleName(void) override;
    POS_EVENT_ID CheckReqSanity(MetaFsRequestBase& reqMsg);

    virtual void InitVolume(MetaVolumeType volType, std::string arrayName, MetaLpnType maxVolPageNum);
    virtual bool OpenVolume(bool isNPOR);
    virtual bool CloseVolume(bool& resetCxt /*output */);
    virtual bool CreateVolume(MetaVolumeType volType);
#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    virtual bool Compaction(bool isNPOR);
#endif

    // API for MetaFs MGMT API (File meta operation, Utility API to obtain specific file meta info.)
    virtual POS_EVENT_ID ProcessNewReq(MetaFsRequestBase& reqMsg);
    virtual bool
    GetVolOpenFlag(std::string arrayName)
    {
        return volContext.GetVolOpenFlag();
    }

    POS_EVENT_ID CheckFileAccessible(FileDescriptorType fd);
    POS_EVENT_ID GetFileSize(FileDescriptorType fd, FileSizeType& outFileByteSize);
    POS_EVENT_ID GetDataChunkSize(FileDescriptorType fd, FileSizeType& outDataChunkSize);
    POS_EVENT_ID GetTargetMediaType(FileDescriptorType fd, MetaStorageType& outTargetMediaType);
    POS_EVENT_ID GetFileBaseLpn(FileDescriptorType fd, MetaLpnType& outFileBaseLpn);
    void SetMss(MetaStorageSubsystem* metaStorage);

private:
    bool _IsVolumeSpecificRequest(MetaFsFileControlType reqType);

    POS_EVENT_ID _HandleVolumeSpcfRequest(MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID _HandleGlobalMetaRequest(MetaFsFileControlRequest& reqMsg);

    void _InitRequestHandler(void);
    int _GetRequestHandlerIndex(MetaFsFileControlType reqType);
    void _RegisterVolumeSpcfReqHandler(MetaFsFileControlType reqType, MetaVolSpcfReqHandler handler);
    void _RegisterGlobalMetaReqHandler(MetaFsFileControlType reqType, GlobalMetaReqHandler handler);

    std::pair<MetaVolumeType, POS_EVENT_ID> _LookupTargetMetaVolume(MetaFsFileControlRequest& reqMsg);
    bool _IsValidVolumeType(MetaVolumeType volType);
    MetaVolSpcfReqHandler _DispatchVolumeSpcfReqHandler(MetaFsFileControlType reqType);
    GlobalMetaReqHandler _DispatchGlobalMetaReqHandler(MetaFsFileControlType reqType);
    POS_EVENT_ID _ExecuteVolumeRequest(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID _ExecuteGlobalMetaRequest(MetaFsFileControlRequest& reqMsg);

    bool _CheckFileCreateReqSanity(MetaVolume& tgtMetaVol, MetaFsFileControlRequest& reqMsg);
    POS_EVENT_ID _CheckSanityBasic(MetaFsFileControlRequest& reqMsg);

    MetaVolSpcfReqHandler volumeSpcfReqHandler[(uint32_t)(MetaFsFileControlType::OnVolumeSpcfReq_Count)];
    GlobalMetaReqHandler globalRequestHandler[(uint32_t)(MetaFsFileControlType::NonVolumeSpcfReq_Count)];

    MetaVolumeHandler volHandler;
    MetaVolumeContext volContext;
    MetaStorageSubsystem* metaStorage;
};
} // namespace pos

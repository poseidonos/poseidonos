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
 * iBoFOS - Meta Filesystem Layer
 * 
 * Meta Volume Manager
*/
#pragma once

#include <string>
#include <utility>

#include "meta_fd_mgr.h"
#include "meta_file_mgr.h"
#include "meta_vol_base.h"
#include "meta_vol_catalog_mgr.h"
#include "meta_vol_context.h"
#include "mf_inode_mgr.h"
#include "mfs_common.h"
#include "mfs_mvm_top.h"
#include "mk/ibof_config.h"
#include "mvm_req.h"
#include "nvram_meta_vol.h"
#include "ssd_meta_vol.h"

class MetaVolMgrClass;
extern MetaVolMgrClass metaVolMgr;
using MetaVolSpcfReqHandler = IBOF_EVENT_ID (MetaVolMgrClass::*)(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
using GlobalMetaReqHandler = IBOF_EVENT_ID (MetaVolMgrClass::*)(MetaFsMoMReqMsg& reqMsg);

class MetaVolMgrClass : public MetaFsMVMTopMgrClass
{
public:
    MetaVolMgrClass(void);
    ~MetaVolMgrClass(void);
    static MetaVolMgrClass& GetInstance(void);

    // APIs for MSC block
    virtual void Init(MetaVolumeType volType, MetaLpnType maxVolPageNum) override;
    virtual bool Bringup(void) override;
    virtual bool Open(bool isNPOR) override;
    virtual bool Close(bool& resetCxt /*output */) override;
    virtual bool CreateVolume(MetaVolumeType volType) override;
#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    virtual bool Compaction(bool isNPOR) override;
#endif
    virtual MetaLpnType GetMaxMetaLpn(MetaVolumeType mediaType) override;

    // API for MetaFs MGMT API (File meta operation, Utility API to obtain specific file meta info.)
    virtual IBOF_EVENT_ID ProcessNewReq(MetaFsMoMReqMsg& reqMsg) override;
    virtual bool
    GetVolOpenFlag(void)
    {
        return volContext.GetVolOpenFlag();
    }

protected:
    virtual bool _IsSiblingModuleReady(void) override;

private:
    bool _IsVolumeSpecificRequest(MetaFsMoMReqType reqType);

    IBOF_EVENT_ID _HandleVolumeSpcfRequest(MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleGlobalMetaRequest(MetaFsMoMReqMsg& reqMsg);

    IBOF_EVENT_ID _HandleOpenFileReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleCloseFileReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleCreateFileReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleDeleteFileReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleGetDataChunkSizeReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleCheckFileAccessibleReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleGetFileSizeReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleGetTargetMediaTypeReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleGetFileBaseLpnReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleFileLockReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleFileUnlockReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleGetFreeFileRegionSizeReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleCheckFileExist(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    // WBT
    IBOF_EVENT_ID _HandleGetMetaFileInodeListReq(MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleGetMaxFileSizeLimitReq(MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleGetFileInodeReq(MetaFsMoMReqMsg& reqMsg);

    IBOF_EVENT_ID _HandleEstimateDataChunkSizeReq(MetaFsMoMReqMsg& reqMsg);

    void _InitReqHandler(void);
    int _GetReqHandlerIdx(MetaFsMoMReqType reqType);
    void _RegisterVolumeSpcfReqHandler(MetaFsMoMReqType reqType, MetaVolSpcfReqHandler handler);
    void _RegisterGlobalMetaReqHandler(MetaFsMoMReqType reqType, GlobalMetaReqHandler handler);

    std::pair<MetaVolumeType, IBOF_EVENT_ID> _LookupTargetMetaVolume(MetaFsMoMReqMsg& reqMsg);
    bool _IsValidVolumeType(MetaVolumeType volType);
    MetaVolSpcfReqHandler _DispatchVolumeSpcfReqHandler(MetaFsMoMReqType reqType);
    GlobalMetaReqHandler _DispatchGlobalMetaReqHandler(MetaFsMoMReqType reqType);
    IBOF_EVENT_ID _ExecuteVolumeRequest(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    IBOF_EVENT_ID _ExecuteGlobalMetaRequest(MetaFsMoMReqMsg& reqMsg);

    bool _CheckFileCreateReqSanity(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);

    MetaVolSpcfReqHandler volumeSpcfReqHandler[(uint32_t)(MetaFsMoMReqType::OnVolumeSpcfReq_Count)];
    GlobalMetaReqHandler globalRequestHandler[(uint32_t)(MetaFsMoMReqType::NonVolumeSpcfReq_Count)];

    MetaVolContext volContext;
};

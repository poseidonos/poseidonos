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
 * MetaFS MVM Top (Abstract class)
*/
#pragma once

#include "meta_vol_base.h"
#include "meta_vol_type.h"
#include "mf_inode.h"
#include "mfs_common.h"
#include "mfs_fb_top.h"
#include "mfs_file_property.h"
#include "mfs_ret_code.h"
#include "mvm_req.h"

class MetaFsMVMTopMgrClass : public MetaFsTopMgrClass
{
public:
    MetaFsMVMTopMgrClass(void);

    virtual const char* GetModuleName(void) override;
    virtual void Init(MetaVolumeType volType, MetaLpnType maxVolPageNum) = 0;
    virtual bool Bringup(void) = 0;
    virtual bool Open(bool isNPOR) = 0;
    virtual bool Close(bool& resetCxt /*output*/) = 0;
    virtual bool CreateVolume(MetaVolumeType volType) = 0;
#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    virtual bool Compaction(bool isNPOR) = 0;
#endif
    virtual MetaLpnType GetMaxMetaLpn(MetaVolumeType mediaType) = 0;

    // util methods for external
    virtual IBOF_EVENT_ID ProcessNewReq(MetaFsMoMReqMsg& reqMsg) = 0;
    virtual IBOF_EVENT_ID CheckReqSanity(MetaFsMoMReqMsg& reqMsg) final;

    // helper functions for mfs internal purpose
    IBOF_EVENT_ID CheckFileAccessible(FileFDType fd);
    IBOF_EVENT_ID GetFileSize(FileFDType fd, FileSizeType& outFileByteSize);
    IBOF_EVENT_ID GetDataChunkSize(FileFDType fd, FileSizeType& outDataChunkSize);
    IBOF_EVENT_ID GetTargetMediaType(FileFDType fd, MetaStorageType& outTargetMediaType);
    IBOF_EVENT_ID GetFileBaseLpn(FileFDType fd, MetaLpnType& outFileBaseLpn);

protected:
    virtual bool _IsSiblingModuleReady(void);

private:
    IBOF_EVENT_ID _CheckSanityBasic(MetaFsMoMReqMsg& reqMsg);
};

extern MetaFsMVMTopMgrClass& mvmTopMgr;

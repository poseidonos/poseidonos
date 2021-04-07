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
 * Mocking Meta Volume Manager
*/
#pragma once

#include "meta_vol_type.h"
#include "mfs_mvm_top.h"
#include "mvm_req.h"

class MockMetaVolMgrClass : public MetaFsMVMTopMgrClass
{
public:
    MockMetaVolMgrClass(void);
    static MockMetaVolMgrClass& GetInstance(void);

    virtual void Init(MetaVolumeType volType, MetaLpnType maxVolPageNum) final;
    virtual bool Bringup(void) final;
    virtual bool Open(bool isNPOR) final;
    virtual bool Close(bool& resetCxt /*output*/) final;
    virtual bool CreateVolume(MetaVolumeType volType) final;

    virtual IBOF_EVENT_ID ProcessNewReq(MetaFsMoMReqMsg& reqMsg) override;

protected:
    uint32_t _GetDefaultDataChunkSize(void);

private:
    IBOF_EVENT_ID _CreateDummyFile(FileSizeType fileByteSize);
    void _CreateFileInode(FileFDType fd, FileSizeType fileByteSize);

    MetaFileInode dummyInode;
};

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
 * MetaFS MIM Top (Abstract class)
*/
#pragma once

#include "mfs_fb_top.h"
#include "mfs_ret_code.h"
#include "mim_req.h"

// Please add methods which are necessary to interact to other modules or to get new command from API wrapper
// Specify all APIs that MIM block needs to provide to upper or other functional blocks
class MetaFsMIMTopMgrClass : public MetaFsTopMgrClass
{
public:
    MetaFsMIMTopMgrClass(void);

    virtual const char* GetModuleName(void) override;
    virtual void Init(void) = 0;
    virtual bool Bringup(void) = 0;
    virtual IBOF_EVENT_ID ProcessNewReq(MetaFsIoReqMsg& reqMsg) = 0;
    virtual void SetMDpageEpochSignature(uint64_t mbrEpochSignature) = 0;
    virtual void Close(void) = 0;
    bool IsValidTagId(uint32_t aioTagId);

    bool IsSuccess(IBOF_EVENT_ID rc);
    IBOF_EVENT_ID CheckReqSanity(MetaFsIoReqMsg& reqMsg);

protected:
    virtual bool _IsSiblingModuleReady(void) override;

private:
    IBOF_EVENT_ID _CheckAIOReqSanity(MetaFsIoReqMsg& reqMsg);
};

extern MetaFsMIMTopMgrClass& mimTopMgr;

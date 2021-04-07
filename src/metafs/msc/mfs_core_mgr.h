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

#pragma once

#include "mfs_msc_top.h"
#include "mfs_ret_code.h"
#include "mfs_state_mgr.h"

class MetaFsCoreMgrClass;
using MetaFsControlReqHandlerPointer = IBOF_EVENT_ID (MetaFsCoreMgrClass::*)(MetaFsControlReqMsg&);

class MetaFsCoreMgrClass : public MetaFsMSCTopMgrClass
{
public:
    MetaFsCoreMgrClass(void);
    ~MetaFsCoreMgrClass(void);
    static MetaFsCoreMgrClass& GetInstance(void);

    virtual bool Init(MetaStorageMediaInfoList& mediaInfoList) override;
    virtual bool Bringup(void) override;
    virtual IBOF_EVENT_ID ProcessNewReq(MetaFsControlReqMsg& reqMsg) override;

    virtual bool IsMounted(void);

private:
    void _InitReqHandler(void);
    void _RegisterReqHandler(MetaFsControlReqType reqType, MetaFsControlReqHandlerPointer handler);
    void _InitiateSystemRecovery(void);

    IBOF_EVENT_ID _HandleFileSysCreateReq(MetaFsControlReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleMountReq(MetaFsControlReqMsg& reqMsg);
    IBOF_EVENT_ID _HandleUnmountReq(MetaFsControlReqMsg& reqMsg);

    MetaFsControlReqHandlerPointer reqHandler[(uint32_t)MetaFsControlReqType::Max];
    MetaFsStateMgrClass mfsStateMgr; // note that stateMgr shouldn't be called by other modules

    bool isMfsUnmounted;
};

extern MetaFsCoreMgrClass mfsCoreMgr;

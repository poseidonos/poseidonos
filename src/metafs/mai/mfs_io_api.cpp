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

#include "mfs_io_api.h"

#include "instance_tagid_allocator.h"
#include "mfs_aiocb_cxt.h"

using namespace ibofos;

static InstanceTagIdAllocator aiocbTagIdAllocator;

MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsIoApiWrapperClass::Read(FileFDType fd, void* buf)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsIoReqMsg reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaIoReqTypeEnum::Read;
    reqMsg.fd = fd;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = true;
    reqMsg.ioMode = MetaIoModeEnum::Sync;
    reqMsg.tagId = aiocbTagIdAllocator();

    rc = mim.HandleNewReq(reqMsg); // MetaIoMgr::_ProcessNewIoReq()

    return rc;
}
MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsIoApiWrapperClass::Read(FileFDType fd, FileSizeType byteOffset, FileSizeType byteSize, void* buf)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsIoReqMsg reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaIoReqTypeEnum::Read;
    reqMsg.fd = fd;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = false;
    reqMsg.ioMode = MetaIoModeEnum::Sync;
    reqMsg.byteOffsetInFile = byteOffset;
    reqMsg.byteSize = byteSize;
    reqMsg.tagId = aiocbTagIdAllocator();

    rc = mim.HandleNewReq(reqMsg); // MetaIoMgr::_ProcessNewIoReq()

    return rc;
}

MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsIoApiWrapperClass::Write(FileFDType fd, void* buf)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsIoReqMsg reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaIoReqTypeEnum::Write;
    reqMsg.fd = fd;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = true;
    reqMsg.ioMode = MetaIoModeEnum::Sync;
    reqMsg.tagId = aiocbTagIdAllocator();

    rc = mim.HandleNewReq(reqMsg); // MetaIoMgr::_ProcessNewIoReq()

    return rc;
}

MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsIoApiWrapperClass::Write(FileFDType fd, FileSizeType byteOffset, FileSizeType byteSize, void* buf)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsIoReqMsg reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaIoReqTypeEnum::Write;
    reqMsg.fd = fd;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = false;
    reqMsg.ioMode = MetaIoModeEnum::Sync;
    reqMsg.byteOffsetInFile = byteOffset;
    reqMsg.byteSize = byteSize;
    reqMsg.tagId = aiocbTagIdAllocator();

    rc = mim.HandleNewReq(reqMsg); // MetaIoMgr::_ProcessNewIoReq()

    return rc;
}

MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsIoApiWrapperClass::SubmitIO(MetaFsAioCbCxt* cxt)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    MetaFsIoReqMsg reqMsg;

    rc.returnData = 0;

    cxt->SetTagId(aiocbTagIdAllocator());

    reqMsg.reqType = (MetaIoReqTypeEnum)cxt->opcode;
    reqMsg.fd = cxt->fd;
    reqMsg.buf = cxt->buf;
    reqMsg.isFullFileIo = (cxt->soffset == 0 && cxt->nbytes == 0);
    reqMsg.ioMode = MetaIoModeEnum::Async;
    reqMsg.byteOffsetInFile = cxt->soffset;
    reqMsg.byteSize = cxt->nbytes;
    reqMsg.aiocb = cxt;
    reqMsg.tagId = cxt->tagId;
    reqMsg.userIo = cxt->userIo;

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[MSG ][SubmitIO   ] type={}, req.tagId={}, fd={}", reqMsg.reqType, reqMsg.tagId, reqMsg.fd);

    rc = mim.HandleNewReq(reqMsg); // MetaIoMgr::_ProcessNewIoReq()

    return rc;
}

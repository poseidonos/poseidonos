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

#include <string>
#include "metafs_io_api.h"
#include "instance_tagid_allocator.h"
#include "metafs_aiocb_cxt.h"

namespace pos
{
static InstanceTagIdAllocator aiocbTagIdAllocator;

MetaFsReturnCode<POS_EVENT_ID>
MetaFsIoApi::Read(FileDescriptorType fd, std::string arrayName, void* buf)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsIoRequest reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaIoRequestType::Read;
    reqMsg.fd = fd;
    reqMsg.arrayName = arrayName;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = true;
    reqMsg.ioMode = MetaIoMode::Sync;
    reqMsg.tagId = aiocbTagIdAllocator();

    rc = ioMgr.HandleNewRequest(reqMsg); // MetaIoManager::_ProcessNewIoReq()

    return rc;
}
MetaFsReturnCode<POS_EVENT_ID>
MetaFsIoApi::Read(FileDescriptorType fd, std::string arrayName,
                FileSizeType byteOffset, FileSizeType byteSize, void* buf)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsIoRequest reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaIoRequestType::Read;
    reqMsg.fd = fd;
    reqMsg.arrayName = arrayName;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = false;
    reqMsg.ioMode = MetaIoMode::Sync;
    reqMsg.byteOffsetInFile = byteOffset;
    reqMsg.byteSize = byteSize;
    reqMsg.tagId = aiocbTagIdAllocator();

    rc = ioMgr.HandleNewRequest(reqMsg); // MetaIoManager::_ProcessNewIoReq()

    return rc;
}

MetaFsReturnCode<POS_EVENT_ID>
MetaFsIoApi::Write(FileDescriptorType fd, std::string arrayName, void* buf)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsIoRequest reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaIoRequestType::Write;
    reqMsg.fd = fd;
    reqMsg.arrayName = arrayName;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = true;
    reqMsg.ioMode = MetaIoMode::Sync;
    reqMsg.tagId = aiocbTagIdAllocator();

    rc = ioMgr.HandleNewRequest(reqMsg); // MetaIoManager::_ProcessNewIoReq()

    return rc;
}

MetaFsReturnCode<POS_EVENT_ID>
MetaFsIoApi::Write(FileDescriptorType fd, std::string arrayName,
                FileSizeType byteOffset, FileSizeType byteSize, void* buf)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsIoRequest reqMsg;

    rc.returnData = 0;

    reqMsg.reqType = MetaIoRequestType::Write;
    reqMsg.fd = fd;
    reqMsg.arrayName = arrayName;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = false;
    reqMsg.ioMode = MetaIoMode::Sync;
    reqMsg.byteOffsetInFile = byteOffset;
    reqMsg.byteSize = byteSize;
    reqMsg.tagId = aiocbTagIdAllocator();

    rc = ioMgr.HandleNewRequest(reqMsg); // MetaIoManager::_ProcessNewIoReq()

    return rc;
}

MetaFsReturnCode<POS_EVENT_ID>
MetaFsIoApi::SubmitIO(MetaFsAioCbCxt* cxt)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    MetaFsIoRequest reqMsg;

    rc.returnData = 0;

    cxt->SetTagId(aiocbTagIdAllocator());

    reqMsg.reqType = (MetaIoRequestType)cxt->opcode;
    reqMsg.fd = cxt->fd;
    reqMsg.arrayName = cxt->arrayName;
    reqMsg.buf = cxt->buf;
    reqMsg.isFullFileIo = (cxt->soffset == 0 && cxt->nbytes == 0);
    reqMsg.ioMode = MetaIoMode::Async;
    reqMsg.byteOffsetInFile = cxt->soffset;
    reqMsg.byteSize = cxt->nbytes;
    reqMsg.aiocb = cxt;
    reqMsg.tagId = cxt->tagId;

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[MSG ][SubmitIO   ] type={}, req.tagId={}, fd={}", reqMsg.reqType, reqMsg.tagId, reqMsg.fd);

    rc = ioMgr.HandleNewRequest(reqMsg); // MetaIoManager::_ProcessNewIoReq()

    return rc;
}
} // namespace pos

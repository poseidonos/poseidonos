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

#include "mim_req.h"

#include "instance_tagid_allocator.h"
#include "mfs_common.h"
#include "os_header.h"

const FileBufType MetaFsIoReqMsg::INVALID_BUF = nullptr;
InstanceTagIdAllocator ioReqTagIdAllocator;

MetaFsIoReqMsg::MetaFsIoReqMsg(void)
: reqType(MetaIoReqTypeEnum::Max),
  ioMode(MetaIoModeEnum::Max),
  isFullFileIo(true),
  fd(MetaFsCommonConst::INVALID_FD),
  buf(INVALID_BUF),
  byteOffsetInFile(INVALID_BYTE_OFFSET),
  byteSize(INVALID_BYTE_SIZE),
  targetMediaType(MetaStorageType::Max),
  aiocb(nullptr),
  tagId(0),
  baseMetaLpn(0),
  originalMsg(nullptr),
  requestCount(0),
  userIo(nullptr),
  retryFlag(false),
  error(0)
{
    ioDone = false;
}

void
MetaFsIoReqMsg::CopyUserReqMsg(const MetaFsIoReqMsg& req)
{
    this->reqType = req.reqType;
    this->ioMode = req.ioMode;
    this->isFullFileIo = req.isFullFileIo;
    this->fd = req.fd;
    this->buf = req.buf;
    this->byteOffsetInFile = req.byteOffsetInFile;
    this->byteSize = req.byteSize;
    this->targetMediaType = req.targetMediaType;
    this->aiocb = req.aiocb;
    this->tagId = req.tagId;
    this->userIo = req.userIo;
    this->baseMetaLpn = req.baseMetaLpn;
    this->ioDone = false;
    this->error = false;

    if (MetaIoModeEnum::Sync == req.ioMode)
    {
        if (nullptr == req.originalMsg)
            this->originalMsg = const_cast<MetaFsIoReqMsg*>(&req);
        else
            this->originalMsg = req.originalMsg;
    }
}

MetaFsIoReqMsg::~MetaFsIoReqMsg(void)
{
}

void
MetaFsIoReqMsg::SetRetryFlag(void)
{
    retryFlag = true;
}

bool
MetaFsIoReqMsg::IsValid(void)
{
    // check initial value first which is invalid
    if (reqType >= MetaIoReqTypeEnum::Max ||
        ioMode >= MetaIoModeEnum::Max ||
        fd == MetaFsCommonConst::INVALID_FD ||
        buf == INVALID_BUF ||
        targetMediaType < MetaStorageType::Max)
    {
        return false;
    }

    // check other value by boundary checking, etc.

    return true;
}

bool
MetaFsIoReqMsg::IsSyncIO(void)
{
    return (ioMode == MetaIoModeEnum::Sync);
}

bool
MetaFsIoReqMsg::IsIoCompleted(void)
{
    return ioDone;
}

bool
MetaFsIoReqMsg::GetError(void)
{
    return error;
}

void
MetaFsIoReqMsg::SetError(bool err)
{
    error = err;
}

void
MetaFsIoReqMsg::SuspendUntilIoCompletion(void)
{
    while (!ioDone)
    {
        usleep(1);
    }

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[MIO ][WaitForDone] type={}, req.tagId={}, io done={}", reqType, tagId, ioDone);

    ioDone = false;
}

void
MetaFsIoReqMsg::NotifyIoCompletionToClient(void)
{
    {
        SPIN_LOCK_GUARD_IN_SCOPE(cbLock);
        requestCount--;
        assert(requestCount >= 0);

        if (0 == requestCount)
        {
            ioDone = true;
        }
    }

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[MIO ][NotifyIO   ] NotifyIOCompletionToClient tagId = {}", tagId);
}

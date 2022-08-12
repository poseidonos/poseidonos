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

#include "metafs_io_request.h"

#include <string>

#include "instance_tagid_allocator.h"
#include "metafs_common.h"
#include "os_header.h"

namespace pos
{
const FileBufType MetaFsIoRequest::INVALID_BUF = nullptr;
InstanceTagIdAllocator ioReqTagIdAllocator;

MetaFsIoRequest::MetaFsIoRequest(const uint32_t numaId)
: reqType(MetaIoRequestType::Max),
  ioMode(MetaIoMode::Max),
  isFullFileIo(true),
  fd(MetaFsCommonConst::INVALID_FD),
  arrayId(INT32_MAX),
  buf(INVALID_BUF),
  byteOffsetInFile(INVALID_BYTE_OFFSET),
  byteSize(INVALID_BYTE_SIZE),
  targetMediaType(MetaStorageType::Max),
  aiocb(nullptr),
  tagId(0),
  baseMetaLpn(0),
  extents(nullptr),
  extentsCount(0),
  originalMsg(nullptr),
  requestCount(0),
  fileCtx(nullptr),
  priority(RequestPriority::Normal),
  numaId(numaId),
  retryFlag(false),
  ioDone(false),
  error(0)
{
    StoreTimestamp(IoRequestStage::Create);
}

// copy except retryFlag, ioDone and error
MetaFsIoRequest::MetaFsIoRequest(const MetaFsIoRequest& req)
: reqType(req.reqType),
  ioMode(req.ioMode),
  isFullFileIo(req.isFullFileIo),
  fd(req.fd),
  arrayId(req.arrayId),
  buf(req.buf),
  byteOffsetInFile(req.byteOffsetInFile),
  byteSize(req.byteSize),
  targetMediaType(req.targetMediaType),
  aiocb(req.aiocb),
  tagId(req.tagId),
  baseMetaLpn(req.baseMetaLpn),
  extents(req.extents),
  extentsCount(req.extentsCount),
  originalMsg(nullptr),
  requestCount(0),
  fileCtx(req.fileCtx),
  priority(req.priority),
  numaId(req.numaId),
  retryFlag(false),
  ioDone(false),
  error(0)
{
    if (MetaIoMode::Sync == req.ioMode)
    {
        if (nullptr == req.originalMsg)
        {
            // from user thread to meta io scheduler
            this->originalMsg = const_cast<MetaFsIoRequest*>(&req);
        }
        else
        {
            // meta io scheduler to mio handler
            this->originalMsg = req.originalMsg;
        }
    }
}

MetaFsIoRequest::~MetaFsIoRequest(void)
{
}

void
MetaFsIoRequest::SetRetryFlag(void)
{
    retryFlag = true;
}

bool
MetaFsIoRequest::IsValid(void)
{
    // check initial value first which is invalid
    if (reqType >= MetaIoRequestType::Max ||
        ioMode >= MetaIoMode::Max ||
        fd == MetaFsCommonConst::INVALID_FD ||
        buf == INVALID_BUF ||
        targetMediaType >= MetaStorageType::Max)
    {
        return false;
    }

    // check other value by boundary checking, etc.

    return true;
}

bool
MetaFsIoRequest::IsSyncIO(void)
{
    return (ioMode == MetaIoMode::Sync);
}

bool
MetaFsIoRequest::IsIoCompleted(void)
{
    return ioDone;
}

bool
MetaFsIoRequest::GetError(void)
{
    return error;
}

void
MetaFsIoRequest::SetError(bool err)
{
    error = err;
}

void
MetaFsIoRequest::SuspendUntilIoCompletion(void)
{
    while (!ioDone)
    {
        usleep(1);
    }

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "[MIO ][WaitForDone] type={}, req.tagId={}, io done={}", reqType, tagId, ioDone);

    ioDone = false;
}

void
MetaFsIoRequest::NotifyIoCompletionToClient(void)
{
    {
        SPIN_LOCK_GUARD_IN_SCOPE(cbLock);
        requestCount--;
        assert(requestCount >= 0);

        if (0 == requestCount)
        {
            StoreTimestamp(IoRequestStage::Complete);
            ioDone = true;
        }
    }

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "[MIO ][NotifyIO   ] NotifyIOCompletionToClient tagId = {}", tagId);
}

std::string
MetaFsIoRequest::GetLogString(void) const
{
    std::string log;
    log.append("reqType: " + (int)reqType);
    log.append(", ioMode: " + (int)ioMode);
    log.append(", tagId: " + tagId);
    log.append(", fd: " + std::to_string(fd));
    log.append(", targetMediaType: " + (int)targetMediaType);
    log.append(", arrayId: " + std::to_string(arrayId));
    log.append(", byteOffsetInFile: " + std::to_string(byteOffsetInFile));
    log.append(", byteSize: " + std::to_string(byteSize));
    log.append(", priority: " + (int)priority);
    return log;
}

MetaLpnType
MetaFsIoRequest::GetStartLpn(void) const
{
    MetaLpnType start = 0;
    MetaLpnType offsetInLpn = byteOffsetInFile / fileCtx->chunkSize;

    for (int i = 0; i < extentsCount; ++i)
    {
        int64_t result = offsetInLpn - extents[i].GetCount();
        if (result < 0)
        {
            start = extents[i].GetStartLpn() + offsetInLpn;
            break;
        }
        offsetInLpn -= extents[i].GetCount();
    }

    return start;
}

size_t
MetaFsIoRequest::GetRequestLpnCount(void) const
{
    size_t startLpn = byteOffsetInFile / fileCtx->chunkSize;
    size_t endLpn = (byteOffsetInFile + byteSize - 1) / fileCtx->chunkSize;

    return endLpn - startLpn + 1;
}

MetaFileType
MetaFsIoRequest::GetFileType(void) const
{
    return fileCtx->fileType;
}
} // namespace pos

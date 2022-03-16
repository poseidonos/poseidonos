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

#include "metafs_io_api.h"

#include <string>

#include "instance_tagid_allocator.h"
#include "metafs_aiocb_cxt.h"

namespace pos
{
static InstanceTagIdAllocator aiocbTagIdAllocator;

MetaFsIoApi::MetaFsIoApi(void)
: arrayId(INT32_MAX),
  isNormal(false),
  ioMgr(nullptr),
  ctrlMgr(nullptr),
  telemetryPublisher(nullptr)
{
}

MetaFsIoApi::MetaFsIoApi(int arrayId, MetaFsFileControlApi* ctrl,
    MetaStorageSubsystem* storage, TelemetryPublisher* tp, MetaIoManager* io)
: arrayId(arrayId),
  isNormal(false),
  ioMgr(io),
  ctrlMgr(ctrl),
  telemetryPublisher(tp)
{
    if (!ioMgr)
        ioMgr = new MetaIoManager(storage);
}

MetaFsIoApi::~MetaFsIoApi(void)
{
    delete ioMgr;
}

POS_EVENT_ID
MetaFsIoApi::Read(FileDescriptorType fd, void* buf, MetaStorageType mediaType)
{
    if (!isNormal)
        return POS_EVENT_ID::MFS_MODULE_NOT_READY;

    MetaFsIoRequest reqMsg;
    reqMsg.reqType = MetaIoRequestType::Read;
    reqMsg.fd = fd;
    reqMsg.arrayId = arrayId;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = true;
    reqMsg.ioMode = MetaIoMode::Sync;
    reqMsg.tagId = aiocbTagIdAllocator();
    reqMsg.targetMediaType = mediaType;
    reqMsg.priority = RequestPriority::Normal;

    return _ProcessRequest(reqMsg);
}

POS_EVENT_ID
MetaFsIoApi::Read(FileDescriptorType fd, FileSizeType byteOffset,
    FileSizeType byteSize, void* buf, MetaStorageType mediaType)
{
    if (!isNormal)
        return POS_EVENT_ID::MFS_MODULE_NOT_READY;

    MetaFsIoRequest reqMsg;
    reqMsg.reqType = MetaIoRequestType::Read;
    reqMsg.fd = fd;
    reqMsg.arrayId = arrayId;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = false;
    reqMsg.ioMode = MetaIoMode::Sync;
    reqMsg.byteOffsetInFile = byteOffset;
    reqMsg.byteSize = byteSize;
    reqMsg.tagId = aiocbTagIdAllocator();
    reqMsg.targetMediaType = mediaType;
    reqMsg.priority = RequestPriority::Normal;

    return _ProcessRequest(reqMsg);
}

POS_EVENT_ID
MetaFsIoApi::Write(FileDescriptorType fd, void* buf, MetaStorageType mediaType)
{
    if (!isNormal)
        return POS_EVENT_ID::MFS_MODULE_NOT_READY;

    MetaFsIoRequest reqMsg;
    reqMsg.reqType = MetaIoRequestType::Write;
    reqMsg.fd = fd;
    reqMsg.arrayId = arrayId;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = true;
    reqMsg.ioMode = MetaIoMode::Sync;
    reqMsg.tagId = aiocbTagIdAllocator();
    reqMsg.targetMediaType = mediaType;
    reqMsg.priority = RequestPriority::Normal;

    return _ProcessRequest(reqMsg);
}

POS_EVENT_ID
MetaFsIoApi::Write(FileDescriptorType fd, FileSizeType byteOffset,
    FileSizeType byteSize, void* buf, MetaStorageType mediaType)
{
    if (!isNormal)
        return POS_EVENT_ID::MFS_MODULE_NOT_READY;

    MetaFsIoRequest reqMsg;
    reqMsg.reqType = MetaIoRequestType::Write;
    reqMsg.fd = fd;
    reqMsg.arrayId = arrayId;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = false;
    reqMsg.ioMode = MetaIoMode::Sync;
    reqMsg.byteOffsetInFile = byteOffset;
    reqMsg.byteSize = byteSize;
    reqMsg.tagId = aiocbTagIdAllocator();
    reqMsg.targetMediaType = mediaType;
    reqMsg.priority = RequestPriority::Normal;

    return _ProcessRequest(reqMsg);
}

POS_EVENT_ID
MetaFsIoApi::SubmitIO(MetaFsAioCbCxt* cxt, MetaStorageType mediaType)
{
    if (!isNormal)
        return POS_EVENT_ID::MFS_MODULE_NOT_READY;

    cxt->SetTagId(aiocbTagIdAllocator());

    MetaFsIoRequest reqMsg;
    reqMsg.reqType = (MetaIoRequestType)cxt->opcode;
    reqMsg.fd = cxt->fd;
    reqMsg.arrayId = cxt->arrayId;
    reqMsg.buf = cxt->buf;
    reqMsg.isFullFileIo = (cxt->soffset == 0 && cxt->nbytes == 0);
    reqMsg.ioMode = MetaIoMode::Async;
    reqMsg.byteOffsetInFile = cxt->soffset;
    reqMsg.byteSize = cxt->nbytes;
    reqMsg.aiocb = cxt;
    reqMsg.tagId = cxt->tagId;
    reqMsg.targetMediaType = mediaType;
    reqMsg.priority = cxt->GetPriority();

    return _ProcessRequest(reqMsg);
}

bool
MetaFsIoApi::AddArray(int arrayId)
{
    return ioMgr->AddArrayInfo(arrayId);
}

bool
MetaFsIoApi::RemoveArray(int arrayId)
{
    return ioMgr->RemoveArrayInfo(arrayId);
}

void
MetaFsIoApi::SetStatus(bool isNormal)
{
    this->isNormal = isNormal;
}

bool
MetaFsIoApi::_AddFileInfo(MetaFsIoRequest& reqMsg)
{
    MetaFileContext* fileCtx = ctrlMgr->GetFileInfo(reqMsg.fd,
        MetaFileUtil::ConvertToVolumeType(reqMsg.targetMediaType));

    // the file is not existed.
    if (fileCtx == nullptr)
        return false;

    reqMsg.fileCtx = fileCtx;

    return true;
}

void
MetaFsIoApi::_AddExtraIoReqInfo(MetaFsIoRequest& reqMsg)
{
    if (reqMsg.isFullFileIo)
    {
        reqMsg.byteOffsetInFile = 0;
        reqMsg.byteSize = reqMsg.fileCtx->sizeInByte;
    }

    reqMsg.targetMediaType = reqMsg.fileCtx->storageType;
    reqMsg.extentsCount = reqMsg.fileCtx->extentsCount;
    reqMsg.extents = reqMsg.fileCtx->extents;
}

POS_EVENT_ID
MetaFsIoApi::_CheckFileIoBoundary(MetaFsIoRequest& reqMsg)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    FileSizeType fileByteSize = reqMsg.fileCtx->sizeInByte;

    if (reqMsg.isFullFileIo)
    {
        if (reqMsg.byteOffsetInFile != 0 ||
            reqMsg.byteSize != fileByteSize)
        {
            rc = POS_EVENT_ID::MFS_INVALID_PARAMETER;
        }
    }
    else
    {
        if (reqMsg.byteOffsetInFile >= fileByteSize ||
            (reqMsg.byteOffsetInFile + reqMsg.byteSize) > fileByteSize)
        {
            rc = POS_EVENT_ID::MFS_INVALID_PARAMETER;
        }
    }
    return rc;
}

POS_EVENT_ID
MetaFsIoApi::_CheckReqSanity(MetaFsIoRequest& reqMsg)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;

    if (!reqMsg.IsValid())
    {
        return POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    rc = _CheckFileIoBoundary(reqMsg);
    if (POS_EVENT_ID::SUCCESS != rc)
    {
        MFS_TRACE_ERROR((int)rc,
            "I/O boundary error. " + reqMsg.GetLogString());
        return rc;
    }

    if (!reqMsg.fileCtx->isActivated)
    {
        rc = POS_EVENT_ID::MFS_FILE_NOT_FOUND;
        MFS_TRACE_ERROR((int)rc,
            "The File not found. " + reqMsg.GetLogString());
        return rc;
    }

    return rc;
}

POS_EVENT_ID
MetaFsIoApi::_ProcessRequest(MetaFsIoRequest& reqMsg)
{
    if (!_AddFileInfo(reqMsg))
    {
        return POS_EVENT_ID::MFS_FILE_NOT_FOUND;
    }

    _AddExtraIoReqInfo(reqMsg);

    POS_EVENT_ID rc = _CheckReqSanity(reqMsg);

    if (POS_EVENT_ID::SUCCESS == rc)
    {
        size_t byteSize = 0;
        if (reqMsg.ioMode == MetaIoMode::Async)
        {
            MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "[MSG ][SubmitIO   ] " + reqMsg.GetLogString());
            byteSize = reqMsg.byteSize;
        }
        else if (!reqMsg.isFullFileIo)
        {
            byteSize = reqMsg.byteSize;
        }
        else
        {
            byteSize = reqMsg.fileCtx->sizeInByte;
        }

        rc = ioMgr->HandleNewRequest(reqMsg); // MetaIoManager::_ProcessNewIoReq()
        _SendMetric(reqMsg.reqType, reqMsg.fd, byteSize);
    }

    return rc;
}

void
MetaFsIoApi::_SendMetric(MetaIoRequestType ioType, FileDescriptorType fd, size_t byteSize)
{
    // TODO (munseop.lim): need to improve
    // std::string thread_name = std::to_string(sched_getcpu());
    // std::string io_type = (ioType == MetaIoRequestType::Read) ? "read" : "write";
    // std::string array_id = std::to_string(arrayId);
    // std::string file_descriptor = std::to_string(fd);

    // POSMetric metric(TEL40010_METAFS_USER_REQUEST, POSMetricTypes::MT_COUNT);
    // metric.AddLabel("thread_name", thread_name);
    // metric.AddLabel("io_type", io_type);
    // metric.AddLabel("array_id", array_id);
    // metric.AddLabel("fd", file_descriptor);
    // metric.SetCountValue(byteSize);
    // telemetryPublisher->PublishMetric(metric);

    // POSMetric metricCnt(TEL40011_METAFS_USER_REQUEST_CNT, POSMetricTypes::MT_COUNT);
    // metricCnt.AddLabel("thread_name", thread_name);
    // metricCnt.AddLabel("io_type", io_type);
    // metricCnt.AddLabel("array_id", array_id);
    // metricCnt.AddLabel("fd", file_descriptor);
    // metricCnt.SetCountValue(1);
    // telemetryPublisher->PublishMetric(metricCnt);
}
} // namespace pos

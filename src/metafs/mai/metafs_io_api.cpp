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
#include <src/metafs/lib/metafs_tokenizer.h>

namespace pos
{
static InstanceTagIdAllocator aiocbTagIdAllocator;

MetaFsIoApi::MetaFsIoApi(void)
: arrayId(INT32_MAX),
  isNormal(false),
  ioMgr(nullptr),
  ctrlMgr(nullptr),
  telemetryPublisher(nullptr),
  concurrentMetaFsTimeInterval(nullptr)
{
}

MetaFsIoApi::MetaFsIoApi(int arrayId, MetaFsFileControlApi* ctrl,
    MetaStorageSubsystem* storage, TelemetryPublisher* tp, ConcurrentMetaFsTimeInterval* metaFsTimeInterval,
    const bool supportNumaDedicated, MetaIoManager* io)
: arrayId(arrayId),
  isNormal(false),
  ioMgr(io),
  ctrlMgr(ctrl),
  telemetryPublisher(tp),
  concurrentMetaFsTimeInterval(metaFsTimeInterval)
{
    if (!ioMgr)
        ioMgr = new MetaIoManager(supportNumaDedicated, storage);
}

MetaFsIoApi::~MetaFsIoApi(void)
{
    delete ioMgr;
}

POS_EVENT_ID
MetaFsIoApi::Read(FileDescriptorType fd, void* buf, MetaStorageType mediaType)
{
    if (!isNormal)
        return EID(MFS_MODULE_NOT_READY);

    MetaFsIoRequest reqMsg;
    reqMsg.reqType = MetaIoRequestType::Read;
    reqMsg.fd = fd;
    reqMsg.arrayId = arrayId;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = true;
    reqMsg.ioMode = MetaIoMode::Sync;
    reqMsg.tagId = aiocbTagIdAllocator();
    reqMsg.targetMediaType = mediaType;

    return _ProcessRequest(reqMsg);
}

POS_EVENT_ID
MetaFsIoApi::Read(FileDescriptorType fd, FileSizeType byteOffset,
    FileSizeType byteSize, void* buf, MetaStorageType mediaType)
{
    if (!isNormal)
        return EID(MFS_MODULE_NOT_READY);

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

    return _ProcessRequest(reqMsg);
}

POS_EVENT_ID
MetaFsIoApi::Write(FileDescriptorType fd, void* buf, MetaStorageType mediaType)
{
    if (!isNormal)
        return EID(MFS_MODULE_NOT_READY);

    MetaFsIoRequest reqMsg;
    reqMsg.reqType = MetaIoRequestType::Write;
    reqMsg.fd = fd;
    reqMsg.arrayId = arrayId;
    reqMsg.buf = buf;
    reqMsg.isFullFileIo = true;
    reqMsg.ioMode = MetaIoMode::Sync;
    reqMsg.tagId = aiocbTagIdAllocator();
    reqMsg.targetMediaType = mediaType;

    return _ProcessRequest(reqMsg);
}

POS_EVENT_ID
MetaFsIoApi::Write(FileDescriptorType fd, FileSizeType byteOffset,
    FileSizeType byteSize, void* buf, MetaStorageType mediaType)
{
    if (!isNormal)
        return EID(MFS_MODULE_NOT_READY);

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

    return _ProcessRequest(reqMsg);
}

POS_EVENT_ID
MetaFsIoApi::SubmitIO(MetaFsAioCbCxt* cxt, MetaStorageType mediaType)
{
    if (!isNormal)
        return EID(MFS_MODULE_NOT_READY);

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

    return _ProcessRequest(reqMsg);
}

bool
MetaFsIoApi::AddArray(const int arrayId, const MaxMetaLpnMapPerMetaStorage& map)
{
    return ioMgr->AddArrayInfo(arrayId, map);
}

bool
MetaFsIoApi::RemoveArray(const int arrayId)
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
    POS_EVENT_ID rc = EID(SUCCESS);
    FileSizeType fileByteSize = reqMsg.fileCtx->sizeInByte;

    if (reqMsg.isFullFileIo)
    {
        if (reqMsg.byteOffsetInFile != 0 ||
            reqMsg.byteSize != fileByteSize)
        {
            rc = EID(MFS_INVALID_PARAMETER);
        }
    }
    else
    {
        if (reqMsg.byteOffsetInFile >= fileByteSize ||
            (reqMsg.byteOffsetInFile + reqMsg.byteSize) > fileByteSize)
        {
            rc = EID(MFS_INVALID_PARAMETER);
        }
    }
    return rc;
}

POS_EVENT_ID
MetaFsIoApi::_CheckReqSanity(MetaFsIoRequest& reqMsg)
{
    POS_EVENT_ID rc = EID(SUCCESS);

    if (!reqMsg.IsValid())
    {
        return EID(MFS_INVALID_PARAMETER);
    }

    rc = _CheckFileIoBoundary(reqMsg);
    if (EID(SUCCESS) != rc)
    {
        MFS_TRACE_ERROR((int)rc,
            "I/O boundary error. " + reqMsg.GetLogString());
        return rc;
    }

    if (!reqMsg.fileCtx->isActivated)
    {
        rc = EID(MFS_FILE_NOT_FOUND);
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
        return EID(MFS_FILE_NOT_FOUND);
    }

    _AddExtraIoReqInfo(reqMsg);

    POS_EVENT_ID rc = _CheckReqSanity(reqMsg);

    if (EID(SUCCESS) == rc)
    {
        size_t byteSize = 0;
        if (reqMsg.ioMode == MetaIoMode::Async)
        {
            MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
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
        _SendPeriodicMetrics(reqMsg.reqType, reqMsg.fd, byteSize);
    }

    return rc;
}

void
MetaFsIoApi::_SendPeriodicMetrics(MetaIoRequestType ioType, FileDescriptorType fd, size_t byteSize)
{
    // TODO(sang7.park) : find if there is better way to construct key for map
    std::string thread_name = std::to_string(sched_getcpu());
    std::string io_type = (ioType == MetaIoRequestType::Read) ? "read" : "write";
    std::string array_id = std::to_string(arrayId);
    std::string file_descriptor = std::to_string(fd);
    std::string key_string = thread_name + ":" + io_type + " : " + array_id + ":" + file_descriptor;

    if (concurrentMetaFsTimeInterval->CheckInterval())
    {
        MetaFsTokenizer metaFsTokenizer;
        POSMetricVector* metricList = new POSMetricVector();
        std::string labels[4];
        for (auto it = collectedMetricsMap.begin(); it != collectedMetricsMap.end(); ++it)
        {
            std::string existKeyString = it->first;
            pair<int, int> existValue = it->second;

            metaFsTokenizer.SplitFourStringByColon(existKeyString, labels);

            POSMetric metric(TEL40010_METAFS_USER_REQUEST, POSMetricTypes::MT_COUNT);
            metric.AddLabel("thread_name", labels[0]);
            metric.AddLabel("io_type", labels[1]);
            metric.AddLabel("array_id", labels[2]);
            metric.AddLabel("fd", labels[3]);

            POSMetric metricCnt(TEL40011_METAFS_USER_REQUEST_CNT, POSMetricTypes::MT_COUNT);
            metricCnt.AddLabel("thread_name", labels[0]);
            metricCnt.AddLabel("io_type", labels[1]);
            metricCnt.AddLabel("array_id", labels[2]);
            metricCnt.AddLabel("fd", labels[3]);

            // Check current iteration key is same with key given by parameter
            if (existKeyString == key_string)
            {
                metric.SetCountValue(existValue.first + byteSize);
                metricCnt.SetCountValue(existValue.second + 1);
            }
            else
            {
                metric.SetCountValue(existValue.first);
                metricCnt.SetCountValue(existValue.second);
            }
            metricList->push_back(metric);
            metricList->push_back(metricCnt);

            it->second.first = 0;
            it->second.second = 0;
        }
        POSMetric metricPublishCnt(TEL40012_METAFS_USER_REQUEST_PUBLISH_CNT_PER_INTERVAL, POSMetricTypes::MT_COUNT);
        metricPublishCnt.SetCountValue(1);
        metricList->push_back(metricPublishCnt);
        telemetryPublisher->PublishMetricList(metricList);
    }
    else
    {
        // If key does not exist, insert
        auto result = collectedMetricsMap.insert({key_string, {byteSize, 1}});

        // If key exist, update
        if (result.second == false)
        {
            result.first->second.first += byteSize;
            result.first->second.second += 1;
        }
    }
}
} // namespace pos

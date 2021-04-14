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

#include <functional>

#include "../log/log_handler.h"
#include "src/bio/volume_io.h"
#include "src/mapper/include/mpage_info.h"
#include "src/meta_file_intf/async_context.h"
#include "src/event_scheduler/event.h"

namespace pos
{
class VolumeIo;
class LogBufferWriteDoneNotifier;

using JournalInternalEventCallback = std::function<void(int)>;

class LogWriteContext : public AsyncMetaFileIoCtx
{
public:
    LogWriteContext(void);
    explicit LogWriteContext(LogHandlerInterface* log);
    virtual ~LogWriteContext(void);

    LogHandlerInterface* GetLog(void);

    virtual uint32_t GetLogSize(void);
    virtual int GetLogGroupId(void);

    virtual void SetAllocated(int groupId, uint32_t seqNum, MetaIoCbPtr cb);
    virtual void SetIoRequest(MetaFsIoOpcode op, int fileDescriptor, uint64_t offset);

    virtual void LogWriteDone(void) = 0;

private:
    LogHandlerInterface* log;
    int logGroupId;

    static const uint32_t INVALID_LOG_INDEX = UINT32_MAX;
};

class MapUpdateLogWriteContext : public LogWriteContext
{
public:
    MapUpdateLogWriteContext(LogHandlerInterface* log, MapPageList dirtyList,
        EventSmartPtr callback, LogBufferWriteDoneNotifier* target);
    virtual ~MapUpdateLogWriteContext(void) = default;

    MapPageList& GetDirtyList(void);
    virtual void LogWriteDone(void);

protected:
    LogBufferWriteDoneNotifier* logFilledNotifier;

    MapPageList dirty;
    EventSmartPtr callbackEvent;
};

class BlockMapUpdatedLogWriteContext : public MapUpdateLogWriteContext
{
public:
    BlockMapUpdatedLogWriteContext(void) = delete;
    BlockMapUpdatedLogWriteContext(LogHandlerInterface* log, MapPageList dirty,
        EventSmartPtr callbackEvent, LogBufferWriteDoneNotifier* target);
    virtual ~BlockMapUpdatedLogWriteContext(void) = default;
};

class StripeMapUpdatedLogWriteContext : public MapUpdateLogWriteContext
{
public:
    StripeMapUpdatedLogWriteContext(void) = delete;
    StripeMapUpdatedLogWriteContext(LogHandlerInterface* log, MapPageList dirty,
        EventSmartPtr callbackEvent, LogBufferWriteDoneNotifier* target);
    virtual ~StripeMapUpdatedLogWriteContext(void) = default;
};

class VolumeDeletedLogWriteContext : public LogWriteContext
{
public:
    VolumeDeletedLogWriteContext(int volumeId, LogHandlerInterface* log, JournalInternalEventCallback callback);
    virtual ~VolumeDeletedLogWriteContext(void) = default;

    virtual void LogWriteDone(void) override;

private:
    int volumeId;
    JournalInternalEventCallback callerCallback;
};

class JournalResetContext : public AsyncMetaFileIoCtx
{
public:
    JournalResetContext(int logGroupId, JournalInternalEventCallback callback);
    virtual ~JournalResetContext(void) = default;

    void SetIoRequest(MetaFsIoOpcode op, int fileDescriptor, uint64_t offset,
        uint64_t len, char* buf, MetaIoCbPtr cb);
    void ResetDone(void);

private:
    int logGroupId;
    JournalInternalEventCallback resetCallback;
};

} // namespace pos

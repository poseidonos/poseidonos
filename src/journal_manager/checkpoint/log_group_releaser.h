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

#include <list>
#include <mutex>

#include "checkpoint_observer.h"

namespace ibofos
{
class JournalLogBuffer;
class CheckpointHandler;
class LogWriteHandler;
class DirtyMapManager;
class LogBufferWriteDoneNotifier;

class LogGroupReleaser : public CheckpointObserver
{
public:
    LogGroupReleaser(void);
    virtual ~LogGroupReleaser(void);

    void Init(LogBufferWriteDoneNotifier* notified, JournalLogBuffer* logBuffer,
        LogWriteHandler* logWriteHandler, DirtyMapManager* dirtyPage);
    void Reset(void);

    void AddToFullLogGroup(int groupId);

    int GetNumFullLogGroups(void);
    int GetFlushingLogGroupId(void);

    int StartCheckpoint(void);
    virtual void CheckpointCompleted(void);

protected:
    void _AddToFullLogGroupList(int groupId);
    bool _HasFullLogGroup(void);

    virtual void _FlushNextLogGroup(void);
    void _UpdateFlushingLogGroup(void);
    int _PopFullLogGroup(void);

    void _LogGroupResetCompleted(int logGroupId);

    void _ResetFlushingLogGroup(void);

    LogBufferWriteDoneNotifier* releaseNotifier;

    JournalLogBuffer* logBuffer;
    LogWriteHandler* logWriteHandler;
    DirtyMapManager* dirtyPageManager;

    std::mutex fullLogGroupLock;
    std::list<int> fullLogGroup;

    std::mutex flushLock;
    int flushingLogGroupId;

    CheckpointHandler* checkpointHandler;
};

} // namespace ibofos

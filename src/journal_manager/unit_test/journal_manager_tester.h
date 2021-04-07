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

#include "../checkpoint/log_group_releaser.h"
#include "../journal_manager.h"
#include "../log_write/buffer_offset_allocator.h" // TODO(huijeong.kim) to only include status
#include "allocator_mock.h"
#include "array_mock.h"
#include "mapper_mock.h"

using namespace ibofos;

class JournalManagerTester : public JournalManager
{
public:
    JournalManagerTester(Mapper* mapper, Allocator* allocator, Array* array);
    virtual ~JournalManagerTester(void);

    int Init(void) override;
    void Delete(void);

    void StartCheckpoint(void);
    void SetTriggerCheckpoint(bool val);
    bool IsCheckpointEnabled(void);

    uint32_t GetLogBufferSize(void);
    uint32_t GetLogGroupSize(void);

    void ReinitializeLogBuffer(uint32_t logBufferSize);

    int GetNumFullLogGroups(void);
    LogGroupBufferStatus GetFlushingLogGroupStatus(void);
    int GetNumDirtyMap(int logGroupId);

    int GetLogs(LogList& logList);
    uint32_t GetNumLogsAdded(void);

    bool VolumeDeleted(int volId);

    void SetEnable(bool value);

private:
    int _SetupLogBuffer(void);
    int _GetLogsFromBuffer(LogList& logList);

    Mapper* mapperToUse;
    Allocator* allocatorToUse;
    Array* arrayToUse;
};

class LogGroupReleaserTester : public LogGroupReleaser
{
public:
    LogGroupReleaserTester(Mapper* mapper, Allocator* allocator);
    void InitTester(void);

    bool triggerCheckpoint = true;

protected:
    virtual void _FlushNextLogGroup(void) override;

private:
    Mapper* mapperToUse;
    Allocator* allocatorToUse;
};

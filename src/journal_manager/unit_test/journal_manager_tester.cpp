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

#include "journal_manager_tester.h"

#include <thread>

#include "../checkpoint/checkpoint_handler.h"
#include "../checkpoint/dirty_map_manager.h"
#include "../log/log_buffer_parser.h"
#include "../log_buffer/journal_log_buffer.h"
#include "../log_write/journal_volume_event_handler.h"
#include "../replay/replay_handler.h"

JournalManagerTester::JournalManagerTester(Mapper* mapper, Allocator* allocator, Array* array)
{
    delete logGroupReleaser;
    logGroupReleaser = new LogGroupReleaserTester(mapper, allocator);

    mapperToUse = mapper;
    allocatorToUse = allocator;
    arrayToUse = array;
}

JournalManagerTester::~JournalManagerTester(void)
{
}

int
JournalManagerTester::Init(void)
{
    int ret = 0;

    replayHandler->SetMapperToUse(mapperToUse);
    replayHandler->SetAllocatorToUse(allocatorToUse);
    replayHandler->SetArrayToUse(arrayToUse);

    ret = JournalManager::Init();
    if (ret < 0)
    {
        return ret;
    }

    LogGroupReleaserTester* releaserTester =
        dynamic_cast<LogGroupReleaserTester*>(logGroupReleaser);
    if (releaserTester != nullptr)
    {
        releaserTester->InitTester();
    }

    return ret;
}

void
JournalManagerTester::Delete(void)
{
    logBuffer->Delete();
}

void
JournalManagerTester::StartCheckpoint(void)
{
    logGroupReleaser->StartCheckpoint();
}

void
JournalManagerTester::SetTriggerCheckpoint(bool val)
{
    ((LogGroupReleaserTester*)(logGroupReleaser))->triggerCheckpoint = val;
}

bool
JournalManagerTester::IsCheckpointEnabled(void)
{
    return ((LogGroupReleaserTester*)logGroupReleaser)->triggerCheckpoint;
}

uint32_t
JournalManagerTester::GetLogBufferSize(void)
{
    return logBuffer->GetLogBufferSize();
}

uint32_t
JournalManagerTester::GetLogGroupSize(void)
{
    return logBuffer->GetLogGroupSize();
}

void
JournalManagerTester::ReinitializeLogBuffer(uint32_t logBufferSize)
{
    logBuffer->SetSize(logBufferSize);
}

int
JournalManagerTester::GetNumFullLogGroups(void)
{
    return logGroupReleaser->GetNumFullLogGroups();
}

LogGroupBufferStatus
JournalManagerTester::GetFlushingLogGroupStatus(void)
{
    int groupId = logGroupReleaser->GetFlushingLogGroupId();
    if (groupId == -1)
    {
        return LogGroupBufferStatus::INVALID;
    }
    else
    {
        return bufferAllocator->GetStatus(groupId);
    }
}

int
JournalManagerTester::GetNumDirtyMap(int logGroupId)
{
    return dirtyMapManager->GetDirtyList(logGroupId).size();
}

int
JournalManagerTester::GetLogs(LogList& logList)
{
    if (config->IsEnabled() == false)
    {
        int result = _SetupLogBuffer();
        if (result != 0)
        {
            return result;
        }
    }

    return _GetLogsFromBuffer(logList);
}

int
JournalManagerTester::_SetupLogBuffer(void)
{
    int result = logBuffer->Setup();
    if (result == 0)
    {
        result = logBuffer->SyncResetAll();
    }

    return result;
}

int
JournalManagerTester::_GetLogsFromBuffer(LogList& logList)
{
    LogBufferParser parser;

    int result = 0;
    int groupSize = logBuffer->GetLogGroupSize();
    void* logGroupBuffer = malloc(groupSize);
    for (int groupId = 0; groupId < logBuffer->GetNumLogGroups(); groupId++)
    {
        result = logBuffer->ReadLogBuffer(groupId, logGroupBuffer);
        if (result != 0)
        {
            break;
        }

        LogList logs;
        result = parser.GetLogs(logGroupBuffer, groupSize, logs);
        if (result != 0)
        {
            break;
        }
        logList.merge(logs);
    }
    free(logGroupBuffer);

    return result;
}

uint32_t
JournalManagerTester::GetNumLogsAdded(void)
{
    return bufferAllocator->GetNumLogsAdded();
}

bool
JournalManagerTester::VolumeDeleted(int volId)
{
    bool ret = true;
    ret = volumeEventHandler->VolumeUnmounted("", volId);
    ret = volumeEventHandler->VolumeDeleted("", volId, 0);

    return ret;
}

void
JournalManagerTester::SetEnable(bool value)
{
    config->SetEnable(value);
}

LogGroupReleaserTester::LogGroupReleaserTester(Mapper* mapper, Allocator* allocator)
{
    mapperToUse = mapper;
    allocatorToUse = allocator;
}

void
LogGroupReleaserTester::InitTester(void)
{
    checkpointHandler->SetMapperToUse(mapperToUse);
    checkpointHandler->SetAllocatorToUse(allocatorToUse);
}

void
LogGroupReleaserTester::_FlushNextLogGroup(void)
{
    if (triggerCheckpoint == true)
    {
        LogGroupReleaser::_FlushNextLogGroup();
    }
}

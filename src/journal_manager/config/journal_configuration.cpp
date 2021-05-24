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

#include "journal_configuration.h"

#include <iostream>
#include <iomanip>
#include <string>

#include "src/metafs/include/metafs_service.h"
#include "mk/ibof_config.h"
#include "src/include/pos_event_id.h"
#include "src/include/memory.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#ifndef IBOF_CONFIG_USE_MOCK_FS
#include "src/metafs/metafs_file_intf.h"
#endif

namespace pos
{
JournalConfiguration::JournalConfiguration(std::string arrayName)
: journalEnabled(false),
  logBufferSizeInConfig(UINT64_MAX),
  metaPageSize(UINT64_MAX),
  maxPartitionSize(UINT64_MAX),
  debugEnabled(false),
  numLogGroups(2),
  logBufferSize(UINT64_MAX),
  arrayName(arrayName)
{
    _ReadConfiguration();
}

JournalConfiguration::~JournalConfiguration(void)
{
}

void
JournalConfiguration::Init(void)
{
    _ReadMetaFsConfiguration();
    _ConfigureLogBufferSize();
}

bool
JournalConfiguration::IsEnabled(void)
{
    return journalEnabled;
}

bool
JournalConfiguration::IsDebugEnabled(void)
{
    return debugEnabled;
}

int
JournalConfiguration::GetNumLogGroups(void)
{
    return numLogGroups;
}

uint64_t
JournalConfiguration::GetLogBufferSize(void)
{
    return logBufferSize;
}

uint64_t
JournalConfiguration::GetLogGroupSize(void)
{
    return logBufferSize / numLogGroups;
}

uint64_t
JournalConfiguration::GetMetaPageSize(void)
{
    return metaPageSize;
}

LogGroupLayout
JournalConfiguration::GetLogBufferLayout(int groupId)
{
    return bufferLayout.GetLayout(groupId);
}

void
JournalConfiguration::UpdateLogBufferSize(uint64_t size)
{
    logBufferSize = size;
    bufferLayout.Init(size, numLogGroups);

    int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_CONFIGURATION);
    std::ostringstream os;
    os << "Log buffer is loaded, size is " << logBufferSize;

    POS_TRACE_INFO(eventId, os.str());
    POS_TRACE_INFO_IN_MEMORY(ModuleInDebugLogDump::JOURNAL, eventId, os.str());
}

void
JournalConfiguration::_ReadConfiguration(void)
{
    int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_CONFIGURATION);

    journalEnabled = _IsJournalEnabled();
    if (journalEnabled == true)
    {
        std::ostringstream os;
        os << "Journal is enabled";

        POS_TRACE_INFO(eventId, os.str());
        POS_TRACE_INFO_IN_MEMORY(ModuleInDebugLogDump::JOURNAL, eventId, os.str());

        debugEnabled = _IsDebugEnabled();
        logBufferSizeInConfig = _ReadLogBufferSize();
    }
    else
    {
        std::ostringstream os;
        os << "Journal is disabled" << logBufferSize;
        POS_TRACE_INFO(eventId, os.str());
        POS_TRACE_INFO_IN_MEMORY(ModuleInDebugLogDump::JOURNAL, eventId, os.str());
    }
}

bool
JournalConfiguration::_IsJournalEnabled(void)
{
    bool enabled = false;
    int ret = ConfigManagerSingleton::Instance()->GetValue("journal", "enable",
        static_cast<void*>(&enabled), CONFIG_TYPE_BOOL);
    if (ret != 0)
    {
        enabled = false;
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::JOURNAL_CONFIGURATION),
            "Failed to read journal enablement from config file");
    }
    return enabled;
}

bool
JournalConfiguration::_IsDebugEnabled(void)
{
    bool enabled = false;
    int ret = ConfigManagerSingleton::Instance()->GetValue("journal", "debug_mode",
        &enabled, ConfigType::CONFIG_TYPE_BOOL);

    if (ret == 0)
    {
        if (enabled == true)
        {
            POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::JOURNAL_CONFIGURATION),
                "Journal debug mode enabled");
            return true;
        }
    }

    return false;
}

uint64_t
JournalConfiguration::_ReadLogBufferSize(void)
{
    uint64_t size = 0;
    int ret = ConfigManagerSingleton::Instance()->GetValue("journal", "buffer_size_in_mb",
        static_cast<void*>(&size), ConfigType::CONFIG_TYPE_UINT64);
    if (ret == 0)
    {
        size *= SIZE_MB;
    }
    else
    {
        size = 0;
    }
    return size;
}

void
JournalConfiguration::_ReadMetaFsConfiguration(void)
{
#ifndef IBOF_CONFIG_USE_MOCK_FS
    MetaFilePropertySet prop;
    prop.ioAccPattern = MetaFileAccessPattern::ByteIntensive;
    prop.ioOpType = MetaFileDominant::WriteDominant;
    prop.integrity = MetaFileIntegrityType::Lvl0_Disable;

    metaPageSize = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName)->ctrl->EstimateAlignedFileIOSize(prop);
    maxPartitionSize = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName)->ctrl->GetTheBiggestExtentSize(prop);
#endif
}

void
JournalConfiguration::_ConfigureLogBufferSize(void)
{
    int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_CONFIGURATION);

    if (maxPartitionSize <= metaPageSize)
    {
        logBufferSize = 0;

        POS_TRACE_DEBUG(eventId, "No enugh space to create new log buffer");
        return;
    }

    if (logBufferSizeInConfig == 0 || logBufferSizeInConfig > maxPartitionSize)
    {
        logBufferSize = _GetAlignedSize(maxPartitionSize);
    }
    else
    {
        logBufferSize = _GetAlignedSize(logBufferSizeInConfig);
    }

    bufferLayout.Init(logBufferSize, numLogGroups);

    POS_TRACE_INFO(eventId, "Log buffer size is configured to {}", logBufferSize);
}

uint64_t
JournalConfiguration::_GetAlignedSize(uint64_t size)
{
    // Log group size should be aligned with meta page size
    return AlignDown(size - metaPageSize, metaPageSize * numLogGroups);
}
} // namespace pos

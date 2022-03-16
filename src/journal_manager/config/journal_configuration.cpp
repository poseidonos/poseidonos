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

#include "journal_configuration.h"

#include <iostream>
#include <iomanip>
#include <string>

#include "src/master_context/config_manager.h"
#include "src/metafs/include/metafs_service.h"
#include "mk/ibof_config.h"
#include "src/include/pos_event_id.h"
#include "src/include/memory.h"
#include "src/logger/logger.h"
#ifndef IBOF_CONFIG_USE_MOCK_FS
#include "src/metafs/metafs_file_intf.h"
#endif

namespace pos
{
JournalConfiguration::JournalConfiguration(void)
: JournalConfiguration(ConfigManagerSingleton::Instance())
{
}

// Constructor for unit test
JournalConfiguration::JournalConfiguration(ConfigManager* configManager)
: journalEnabled(false),
  logBufferSizeInConfig(UINT64_MAX),
  metaPageSize(UINT64_MAX),
  maxPartitionSize(UINT64_MAX),
  debugEnabled(false),
  configManager(configManager),
  numLogGroups(2),
  logBufferSize(UINT64_MAX)
{
    _ReadConfiguration();
}

JournalConfiguration::~JournalConfiguration(void)
{
}

void
JournalConfiguration::Init(bool isWriteThroughEnabled)
{
    metaVolumeToUse = (isWriteThroughEnabled == true) ? (MetaVolumeType::JournalVolume) : (MetaVolumeType::NvRamVolume);
    POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::JOURNAL_CONFIGURATION), "Journal will be stored on {}", metaVolumeToUse);
}

int
JournalConfiguration::SetLogBufferSize(uint64_t loadedLogBufferSize, MetaFsFileControlApi* metaFsCtrl)
{
    int result = 0;
    _ReadMetaFsConfiguration(metaFsCtrl);

    if (loadedLogBufferSize == 0)
    {
        uint64_t size = 0;
        result = _ConfigureLogBufferSize(size);
        if (result == 0)
        {
            _SetLogBufferSize(size);
        }
    }
    else
    {
        _SetLogBufferSize(loadedLogBufferSize);
    }
    return result;
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

MetaVolumeType
JournalConfiguration::GetMetaVolumeToUse(void)
{
    return metaVolumeToUse;
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
    int ret = configManager->GetValue("journal", "enable",
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
    int ret = configManager->GetValue("journal", "debug_mode",
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
    int ret = configManager->GetValue("journal", "buffer_size_in_mb",
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
JournalConfiguration::_ReadMetaFsConfiguration(MetaFsFileControlApi* metaFsCtrl)
{
    MetaFilePropertySet prop;
    prop.ioAccPattern = MetaFileAccessPattern::ByteIntensive;
    prop.ioOpType = MetaFileDominant::WriteDominant;
    prop.integrity = MetaFileIntegrityType::Lvl0_Disable;

    metaPageSize = metaFsCtrl->EstimateAlignedFileIOSize(prop, metaVolumeToUse);
    maxPartitionSize = metaFsCtrl->GetAvailableSpace(prop, metaVolumeToUse);
}

int
JournalConfiguration::_ConfigureLogBufferSize(uint64_t& size)
{
    int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_CONFIGURATION);

    if (maxPartitionSize <= metaPageSize)
    {
        POS_TRACE_DEBUG(eventId, "No enugh space to create new log buffer");
        return -1 * eventId;
    }

    if (logBufferSizeInConfig == 0 || logBufferSizeInConfig > maxPartitionSize)
    {
        size = _GetAlignedSize(maxPartitionSize);
    }
    else
    {
        size = _GetAlignedSize(logBufferSizeInConfig);
    }

    return 0;
}

uint64_t
JournalConfiguration::_GetAlignedSize(uint64_t size)
{
    // Log group size should be aligned with meta page size
    return AlignDown(size - metaPageSize, metaPageSize * numLogGroups);
}

void
JournalConfiguration::_SetLogBufferSize(uint64_t size)
{
    logBufferSize = size;

    bufferLayout.Init(logBufferSize, numLogGroups);
    POS_TRACE_INFO(POS_EVENT_ID::JOURNAL_CONFIGURATION,
        "Log buffer size is configured to {}", logBufferSize);
}
} // namespace pos

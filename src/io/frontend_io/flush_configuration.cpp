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

#include "flush_configuration.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"

namespace pos
{
FlushConfiguration::FlushConfiguration(void)
: FlushConfiguration(ConfigManagerSingleton::Instance())
{
}

FlushConfiguration::FlushConfiguration(ConfigManager* configManager)
: enabled(false),
  internalFlushEnabled(false),
  internalFlushThreshold(100),
  configManager(configManager)
{
    bool flushEnableInConfig = _ReadFlushEnableFromConfig();
    bool journalEnableInConfig = _ReadJournalEnableFromConfig();

    if (journalEnableInConfig == true)
    {
        enabled = false;

        POS_TRACE_INFO(EID(FLUSH_HANDLING_DISABLED),
            "Flush command handling is disabled as journal is enabled");
    }
    else
    {
        enabled = flushEnableInConfig;

        if (enabled == true)
        {
            POS_TRACE_INFO(EID(FLUSH_HANDLING_ENABLED), "Flush command handling is enabled");
            internalFlushThreshold = _ReadFlushInternalThresholdFromConfig();
            internalFlushEnabled = _ReadInternalFlushEnableFromConfig();
        }
        else
        {
            POS_TRACE_INFO(EID(FLUSH_HANDLING_DISABLED), "Flush command handling is disabled");
        }
    }
}

bool
FlushConfiguration::_ReadFlushEnableFromConfig(void)
{
    bool enabled;
    int ret = configManager->GetValue("flush", "enable",
        static_cast<void*>(&enabled), CONFIG_TYPE_BOOL);

    if (ret != 0)
    {
        enabled = false;
    }
    return enabled;
}

bool
FlushConfiguration::_ReadJournalEnableFromConfig(void)
{
    bool enabled;
    int ret = configManager->GetValue("journal", "enable",
        static_cast<void*>(&enabled), CONFIG_TYPE_BOOL);

    if (ret != 0)
    {
        enabled = false;
    }
    return enabled;
}

bool
FlushConfiguration::_ReadInternalFlushEnableFromConfig(void)
{
    bool enabled;
    int ret = configManager->GetValue("flush", "internal_flush_enable",
        static_cast<void*>(&enabled), CONFIG_TYPE_BOOL);

    if (ret != 0)
    {
        enabled = false;
    }
    return enabled;
}

int
FlushConfiguration::_ReadFlushInternalThresholdFromConfig(void)
{
    int threshold;
    int ret = configManager->GetValue("flush", "internal_flush_threshold",
        static_cast<void*>(&threshold), CONFIG_TYPE_INT);

    if (ret != 0)
    {
        threshold = 100;
    }
    return threshold;
}

bool
FlushConfiguration::IsEnabled(void)
{
    return enabled;
}

bool
FlushConfiguration::IsInternalFlushEnabled(void)
{
    return internalFlushEnabled;
}

int
FlushConfiguration::GetInternalFlushThreshold(void)
{
    return internalFlushThreshold;
}

} // namespace pos

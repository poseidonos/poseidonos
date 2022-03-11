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

#include "metafs_config_manager.h"

#include <iomanip>
#include <iostream>
#include <string>

#include "src/metafs/config/metafs_config.h"

namespace pos
{
MetaFsConfigManager::MetaFsConfigManager(ConfigManager* configManager)
: configManager_(configManager),
  mioPoolCapacity_(0),
  mpioPoolCapacity_(0),
  writeMpioEnabled_(false),
  writeMpioCapacity_(0),
  directAccessEnabled_(false),
  timeIntervalInMillisecondsForMetric_(0)
{
    _BuildConfigMap();
}

MetaFsConfigManager::~MetaFsConfigManager(void)
{
}

bool
MetaFsConfigManager::Init(void)
{
    mioPoolCapacity_ = _GetMioPoolCapacity();
    mpioPoolCapacity_ = _GetMpioPoolCapacity();
    writeMpioEnabled_ = _IsWriteMpioCacheEnabled();
    writeMpioCapacity_ = _GetWriteMpioCacheCapacity();
    directAccessEnabled_ = _IsDirectAccessEnabled();
    timeIntervalInMillisecondsForMetric_ = _GetTimeIntervalInMillisecondsForMetric();

    if (!_ValidateConfig())
    {
        POS_TRACE_ERROR(static_cast<int>(POS_EVENT_ID::MFS_INVALID_CONFIG),
            "The config values are invalid.");
        return false;
    }

    return true;
}

void
MetaFsConfigManager::_BuildConfigMap(void)
{
    configMap_.insert({MetaFsConfigType::MioPoolCapacity,
        {"mio_pool_capacity", CONFIG_TYPE_UINT64}});
    configMap_.insert({MetaFsConfigType::MpioPoolCapacity,
        {"mpio_pool_capacity", CONFIG_TYPE_UINT64}});
    configMap_.insert({MetaFsConfigType::WriteMpioCacheEnabled,
        {"write_mpio_cache_enable", CONFIG_TYPE_BOOL}});
    configMap_.insert({MetaFsConfigType::WriteMpioCacheCapacity,
        {"write_mpio_cache_capacity", CONFIG_TYPE_UINT64}});
    configMap_.insert({MetaFsConfigType::DirectAccessForJournalEnabled,
        {"direct_access_for_journal_enable", CONFIG_TYPE_BOOL}});
    configMap_.insert({MetaFsConfigType::TimeIntervalInMillisecondsForMetric,
        {"time_interval_in_milliseconds_for_metric", CONFIG_TYPE_UINT64}});
}

bool
MetaFsConfigManager::_ValidateConfig(void) const
{
    // add more constraint here
    if (timeIntervalInMillisecondsForMetric_ < MetaFsConfig::MIN_TIME_INTERVAL)
    {
        POS_TRACE_ERROR(static_cast<int>(POS_EVENT_ID::MFS_INVALID_CONFIG),
            "The time interval ({}) is not valid.",
            timeIntervalInMillisecondsForMetric_);
        return false;
    }

    return true;
}

size_t
MetaFsConfigManager::_GetMioPoolCapacity(void)
{
    size_t count = 0;
    if (_ReadConfiguration<size_t>(MetaFsConfigType::MioPoolCapacity, &count))
        return 0;

    POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::MFS_INFO_MESSAGE),
        configMap_[MetaFsConfigType::MioPoolCapacity].first + ": " + std::to_string(count));

    return count;
}

size_t
MetaFsConfigManager::_GetMpioPoolCapacity(void)
{
    size_t count = 0;
    if (_ReadConfiguration<size_t>(MetaFsConfigType::MpioPoolCapacity, &count))
        return 0;

    POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::MFS_INFO_MESSAGE),
        configMap_[MetaFsConfigType::MpioPoolCapacity].first + ": " + std::to_string(count));

    return count;
}

bool
MetaFsConfigManager::_IsWriteMpioCacheEnabled(void)
{
    bool enabled = false;
    if (_ReadConfiguration<bool>(MetaFsConfigType::WriteMpioCacheEnabled, &enabled))
        return false;

    POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::MFS_INFO_MESSAGE),
        configMap_[MetaFsConfigType::WriteMpioCacheEnabled].first + (enabled ? " is enabled" : " is disabled"));

    return enabled;
}

size_t
MetaFsConfigManager::_GetWriteMpioCacheCapacity(void)
{
    size_t count = 0;
    if (_ReadConfiguration<size_t>(MetaFsConfigType::WriteMpioCacheCapacity, &count))
        return 0;

    POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::MFS_INFO_MESSAGE),
        configMap_[MetaFsConfigType::WriteMpioCacheCapacity].first + ": " + std::to_string(count));

    return count;
}

bool
MetaFsConfigManager::_IsDirectAccessEnabled(void)
{
    bool enabled = false;
    if (_ReadConfiguration<bool>(MetaFsConfigType::DirectAccessForJournalEnabled, &enabled))
        return false;

    POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::MFS_INFO_MESSAGE),
        configMap_[MetaFsConfigType::DirectAccessForJournalEnabled].first + (enabled ? " is enabled" : " is disabled"));

    return enabled;
}

size_t
MetaFsConfigManager::_GetTimeIntervalInMillisecondsForMetric(void)
{
    size_t count = 0;
    if (_ReadConfiguration<size_t>(MetaFsConfigType::TimeIntervalInMillisecondsForMetric, &count))
        return 0;

    POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::MFS_INFO_MESSAGE),
        configMap_[MetaFsConfigType::TimeIntervalInMillisecondsForMetric].first + ": " + std::to_string(count));

    return count;
}
} // namespace pos


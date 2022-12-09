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
  writeMpioCapacity_(0),
  directAccessEnabled_(false),
  timeIntervalInMillisecondsForMetric_(0),
  samplingSkipCount_(0),
  wrrCountSpecialPurposeMap_(0),
  wrrCountJournal_(0),
  wrrCountMap_(0),
  wrrCountGeneral_(0),
  rocksdbEnabled_(false),
  rocksDbPath_(""),
  supportNumaDedicatedScheduling_(false),
  needToIgnoreNumaDedicatedScheduling_(false)
{
    _BuildConfigMap();
}

MetaFsConfigManager::~MetaFsConfigManager(void)
{
    configMap_.clear();
}

bool
MetaFsConfigManager::Init(void)
{
    mioPoolCapacity_ = _GetMioPoolCapacity();
    mpioPoolCapacity_ = _GetMpioPoolCapacity();
    writeMpioCapacity_ = _GetWriteMpioCacheCapacity();
    directAccessEnabled_ = _IsDirectAccessEnabled();
    timeIntervalInMillisecondsForMetric_ = _GetTimeIntervalInMillisecondsForMetric();
    samplingSkipCount_ = _GetSamplingSkipCount();
    wrrCountSpecialPurposeMap_ = _GetWrrCountSpecialPurposeMap();
    wrrCountJournal_ = _GetWrrCountJournal();
    wrrCountMap_ = _GetWrrCountMap();
    wrrCountGeneral_ = _GetWrrCountGeneral();
    rocksdbEnabled_ = _IsRocksdbEnabled();
    if (rocksdbEnabled_)
    {
        rocksDbPath_ = _GetRocksDbPath();
    }
    supportNumaDedicatedScheduling_ = _IsSupportingNumaDedicatedScheduling();
    needToIgnoreNumaDedicatedScheduling_ = false;

    if (!_ValidateConfig())
    {
        POS_TRACE_ERROR(static_cast<int>(EID(MFS_INVALID_CONFIG)),
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
    configMap_.insert({MetaFsConfigType::WriteMpioCacheCapacity,
        {"write_mpio_cache_capacity", CONFIG_TYPE_UINT64}});
    configMap_.insert({MetaFsConfigType::DirectAccessForJournalEnabled,
        {"direct_access_for_journal_enable", CONFIG_TYPE_BOOL}});
    configMap_.insert({MetaFsConfigType::TimeIntervalInMillisecondsForMetric,
        {"time_interval_in_milliseconds_for_metric", CONFIG_TYPE_UINT64}});
    configMap_.insert({MetaFsConfigType::SamplingSkipCount,
        {"sampling_skip_count", CONFIG_TYPE_UINT64}});
    configMap_.insert({MetaFsConfigType::WrrCountSpecialPurposeMap,
        {"wrr_count_special_purpose_map", CONFIG_TYPE_UINT64}});
    configMap_.insert({MetaFsConfigType::WrrCountJournal,
        {"wrr_count_journal", CONFIG_TYPE_UINT64}});
    configMap_.insert({MetaFsConfigType::WrrCountMap,
        {"wrr_count_map", CONFIG_TYPE_UINT64}});
    configMap_.insert({MetaFsConfigType::WrrCountGeneral,
        {"wrr_count_general", CONFIG_TYPE_UINT64}});
    configMap_.insert({MetaFsConfigType::SupportNumaDedicatedScheduling,
        {"numa_dedicated", CONFIG_TYPE_BOOL}});
}

bool
MetaFsConfigManager::_ValidateConfig(void) const
{
    // add more constraint here
    if (timeIntervalInMillisecondsForMetric_ < MetaFsConfig::MIN_TIME_INTERVAL)
    {
        POS_TRACE_ERROR(static_cast<int>(EID(MFS_INVALID_CONFIG)),
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

    POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
        configMap_[MetaFsConfigType::MioPoolCapacity].first + ": " + std::to_string(count));

    return count;
}

size_t
MetaFsConfigManager::_GetMpioPoolCapacity(void)
{
    size_t count = 0;
    if (_ReadConfiguration<size_t>(MetaFsConfigType::MpioPoolCapacity, &count))
        return 0;

    POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
        configMap_[MetaFsConfigType::MpioPoolCapacity].first + ": " + std::to_string(count));

    return count;
}

size_t
MetaFsConfigManager::_GetWriteMpioCacheCapacity(void)
{
    size_t count = 0;
    if (_ReadConfiguration<size_t>(MetaFsConfigType::WriteMpioCacheCapacity, &count))
        return 0;

    POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
        configMap_[MetaFsConfigType::WriteMpioCacheCapacity].first + ": " + std::to_string(count));

    return count;
}

bool
MetaFsConfigManager::_IsDirectAccessEnabled(void)
{
    bool enabled = false;
    if (_ReadConfiguration<bool>(MetaFsConfigType::DirectAccessForJournalEnabled, &enabled))
        return false;

    POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
        configMap_[MetaFsConfigType::DirectAccessForJournalEnabled].first + (enabled ? " is enabled" : " is disabled"));

    return enabled;
}

size_t
MetaFsConfigManager::_GetTimeIntervalInMillisecondsForMetric(void)
{
    size_t count = 0;
    if (_ReadConfiguration<size_t>(MetaFsConfigType::TimeIntervalInMillisecondsForMetric, &count))
        return 0;

    POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
        configMap_[MetaFsConfigType::TimeIntervalInMillisecondsForMetric].first + ": " + std::to_string(count));

    return count;
}

size_t
MetaFsConfigManager::_GetSamplingSkipCount(void)
{
    size_t count = 0;
    if (_ReadConfiguration<size_t>(MetaFsConfigType::SamplingSkipCount, &count))
        return 0;

    POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
        configMap_[MetaFsConfigType::SamplingSkipCount].first + ": " + std::to_string(count));

    return count;
}

size_t
MetaFsConfigManager::_GetWrrCountSpecialPurposeMap(void)
{
    size_t count = 0;
    if (_ReadConfiguration<size_t>(MetaFsConfigType::WrrCountSpecialPurposeMap, &count))
        return 0;

    POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
        configMap_[MetaFsConfigType::WrrCountSpecialPurposeMap].first + ": " + std::to_string(count));

    return count;
}

size_t
MetaFsConfigManager::_GetWrrCountJournal(void)
{
    size_t count = 0;
    if (_ReadConfiguration<size_t>(MetaFsConfigType::WrrCountJournal, &count))
        return 0;

    POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
        configMap_[MetaFsConfigType::WrrCountJournal].first + ": " + std::to_string(count));

    return count;
}

size_t
MetaFsConfigManager::_GetWrrCountMap(void)
{
    size_t count = 0;
    if (_ReadConfiguration<size_t>(MetaFsConfigType::WrrCountMap, &count))
        return 0;

    POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
        configMap_[MetaFsConfigType::WrrCountMap].first + ": " + std::to_string(count));

    return count;
}

size_t
MetaFsConfigManager::_GetWrrCountGeneral(void)
{
    size_t count = 0;
    if (_ReadConfiguration<size_t>(MetaFsConfigType::WrrCountGeneral, &count))
        return 0;

    POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
        configMap_[MetaFsConfigType::WrrCountGeneral].first + ": " + std::to_string(count));

    return count;
}

bool
MetaFsConfigManager::_IsRocksdbEnabled(void)
{
    bool enabled = false;
    int ret = configManager_->GetValue("meta_rocksdb", "metafs_use_rocksdb",
        static_cast<void*>(&enabled), ConfigType::CONFIG_TYPE_BOOL);

    if (ret == 0)
    {
        if (enabled == true)
        {
            POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
                "RocksDB Metafs is enabled");
            return true;
        }
    }
    return false;
}

std::string
MetaFsConfigManager::_GetRocksDbPath(void)
{
    std::string path = "";
    int ret = configManager_->GetValue("meta_rocksdb", "rocksdb_path",
        static_cast<void*>(&path), ConfigType::CONFIG_TYPE_STRING);

    if (ret == 0)
    {
        POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
            "RocksDB Metafs will be saved in {}", path);
    }
    else
    {
        path = "/etc/pos/POSRaid";
        POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
            "RocksDB Metafs will be saved in default path {}", path);
    }
    return path;
}

bool
MetaFsConfigManager::_IsSupportingNumaDedicatedScheduling(void)
{
    bool enabled = false;
    int ret = configManager_->GetValue("performance",
        "numa_dedicated", &enabled, CONFIG_TYPE_BOOL);
    if (ret)
        return false;

    POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
        configMap_[MetaFsConfigType::SupportNumaDedicatedScheduling].first +
            (enabled ? " is supported" : " is not supported"));

    return enabled;
}
} // namespace pos

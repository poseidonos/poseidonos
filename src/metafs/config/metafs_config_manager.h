/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/include/pos_event_id.h"
#include "src/master_context/config_manager.h"
#include "src/metafs/log/metafs_log.h"

namespace pos
{
enum class MetaFsConfigType
{
    MioPoolCapacity,
    MpioPoolCapacity,
    WriteMpioCacheCapacity,
    DirectAccessForJournalEnabled,
    TimeIntervalInMillisecondsForMetric,
    SamplingSkipCount,
    WrrCountSpecialPurposeMap,
    WrrCountJournal,
    WrrCountMap,
    WrrCountGeneral,
    SupportNumaDedicatedScheduling,
};

class MetaFsConfigManager
{
public:
    // for test
    MetaFsConfigManager(void) = default;
    MetaFsConfigManager(ConfigManager* configManager);
    virtual ~MetaFsConfigManager(void);
    virtual bool Init(void);
    virtual size_t GetMioPoolCapacity(void) const
    {
        return mioPoolCapacity_;
    }
    virtual size_t GetMpioPoolCapacity(void) const
    {
        return mpioPoolCapacity_;
    }
    virtual size_t GetWriteMpioCacheCapacity(void) const
    {
        return writeMpioCapacity_;
    }
    virtual bool IsDirectAccessEnabled(void) const
    {
        return directAccessEnabled_;
    }
    virtual size_t GetTimeIntervalInMillisecondsForMetric(void) const
    {
        return timeIntervalInMillisecondsForMetric_;
    }
    virtual size_t GetSamplingSkipCount(void) const
    {
        return samplingSkipCount_;
    }
    virtual size_t GetWrrCountSpecialPurposeMap(void) const
    {
        return wrrCountSpecialPurposeMap_;
    }
    virtual size_t GetWrrCountJournal(void) const
    {
        return wrrCountJournal_;
    }
    virtual size_t GetWrrCountMap(void) const
    {
        return wrrCountMap_;
    }
    virtual size_t GetWrrCountGeneral(void) const
    {
        return wrrCountGeneral_;
    }
    virtual std::vector<int> GetWrrWeight(void) const
    {
        return {
            (int)wrrCountSpecialPurposeMap_,
            (int)wrrCountJournal_,
            (int)wrrCountMap_,
            (int)wrrCountGeneral_};
    }
    virtual bool IsRocksdbEnabled(void) const
    {
        return rocksdbEnabled_;
    }
    virtual std::string GetRocksDbPath(void) const
    {
        return rocksDbPath_;
    }
    virtual bool IsSupportingNumaDedicatedScheduling(void) const
    {
        return supportNumaDedicatedScheduling_;
    }
    virtual void SetIgnoreNumaDedicatedScheduling(const bool ignore)
    {
        needToIgnoreNumaDedicatedScheduling_ = ignore;
    }
    virtual bool NeedToIgnoreNumaDedicatedScheduling(void) const
    {
        return needToIgnoreNumaDedicatedScheduling_;
    }

protected:
    virtual bool _ValidateConfig(void) const;

private:
    template<typename T>
    int _ReadConfiguration(const MetaFsConfigType type, T* value)
    {
        int ret = configManager_->GetValue("metafs", configMap_.at(type).first,
            (void*)value, (ConfigType)configMap_.at(type).second);

        if (ret)
            POS_TRACE_INFO(static_cast<int>(EID(MFS_INFO_MESSAGE)),
                "Failed to read configuration from config file");

        return ret;
    }
    void _BuildConfigMap(void);
    size_t _GetMioPoolCapacity(void);
    size_t _GetMpioPoolCapacity(void);
    size_t _GetWriteMpioCacheCapacity(void);
    bool _IsDirectAccessEnabled(void);
    size_t _GetTimeIntervalInMillisecondsForMetric(void);
    size_t _GetSamplingSkipCount(void);
    size_t _GetWrrCountSpecialPurposeMap(void);
    size_t _GetWrrCountJournal(void);
    size_t _GetWrrCountMap(void);
    size_t _GetWrrCountGeneral(void);
    bool _IsRocksdbEnabled(void);
    std::string _GetRocksDbPath(void);
    bool _IsSupportingNumaDedicatedScheduling(void);

    std::unordered_map<MetaFsConfigType, std::pair<std::string, int>> configMap_;
    ConfigManager* configManager_;
    size_t mioPoolCapacity_;
    size_t mpioPoolCapacity_;
    size_t writeMpioCapacity_;
    bool directAccessEnabled_;
    size_t timeIntervalInMillisecondsForMetric_;
    size_t samplingSkipCount_;
    size_t wrrCountSpecialPurposeMap_;
    size_t wrrCountJournal_;
    size_t wrrCountMap_;
    size_t wrrCountGeneral_;
    bool rocksdbEnabled_;
    std::string rocksDbPath_;
    bool supportNumaDedicatedScheduling_;
    bool needToIgnoreNumaDedicatedScheduling_;
};

} // namespace pos

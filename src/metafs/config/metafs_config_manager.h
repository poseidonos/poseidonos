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

#pragma once

#include <string>
#include <unordered_map>
#include <utility>

#include "src/include/pos_event_id.h"
#include "src/master_context/config_manager.h"
#include "src/metafs/log/metafs_log.h"

namespace pos
{
enum class MetaFsConfigType
{
    MioPoolCapacity,
    MpioPoolCapacity,
    WriteMpioCacheEnabled,
    WriteMpioCacheCapacity,
    DirectAccessForJournalEnabled,
    TimeIntervalInMillisecondsForMetric
};

class MetaFsConfigManager
{
public:
    MetaFsConfigManager(void) = delete;
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
    virtual bool IsWriteMpioCacheEnabled(void) const
    {
        return writeMpioEnabled_;
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

protected:
    virtual bool _ValidateConfig(void) const;

private:
    template<typename T>
    int _ReadConfiguration(const MetaFsConfigType type, T* value)
    {
        int ret = configManager_->GetValue("metafs", configMap_.at(type).first,
            (void*)value, (ConfigType)configMap_.at(type).second);

        if (ret)
            POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::MFS_INFO_MESSAGE),
                "Failed to read configuration from config file");

        return ret;
    }
    void _BuildConfigMap(void);
    size_t _GetMioPoolCapacity(void);
    size_t _GetMpioPoolCapacity(void);
    bool _IsWriteMpioCacheEnabled(void);
    size_t _GetWriteMpioCacheCapacity(void);
    bool _IsDirectAccessEnabled(void);
    size_t _GetTimeIntervalInMillisecondsForMetric(void);

    std::unordered_map<MetaFsConfigType, std::pair<std::string, int>> configMap_;
    ConfigManager* configManager_;
    size_t mioPoolCapacity_;
    size_t mpioPoolCapacity_;
    bool writeMpioEnabled_;
    size_t writeMpioCapacity_;
    bool directAccessEnabled_;
    size_t timeIntervalInMillisecondsForMetric_;
};

} // namespace pos

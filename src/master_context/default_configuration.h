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

#include <string>
#include <vector>

#include <rapidjson/document.h>
#include "src/lib/singleton.h"

using namespace std;

namespace pos
{
class DefaultConfiguration
{
public:
    DefaultConfiguration(void);

    void Restore(void);
    string
    ConfiguratinDir(void)
    {
        return DefaultConfiguration::CONFIGURATION_PATH;
    }
    string
    ConfigurationFilePath(void)
    {
        return DefaultConfiguration::CONFIGURATION_PATH
            + DefaultConfiguration::CONFIGURATION_NAME;
    }
    string
    DefaultConfigurationFilePath(void)
    {
        return DefaultConfiguration::CONFIGURATION_PATH
            + DefaultConfiguration::DEFAULT_CONFIGURATION_NAME;
    }

private:
    struct ConfigKeyValue
    {
        std::string key;
        std::string value;
    };
    struct ConfigModuleData
    {
        std::string moduleName;
        vector<ConfigKeyValue> keyAndValueList;
    };
    vector<ConfigKeyValue> journalData = {
        {"enable", "false"}
    };
    vector<ConfigKeyValue> loggerData = {
        {"logfile_size_in_mb", "50"},
        {"logfile_rotation_count", "20"},
        {"min_allowable_log_level", "\"debug\""},
        {"deduplication_enabled", "true"},
        {"deduplication_sensitivity_in_msec", "20"}
    };
    vector<ConfigKeyValue> ioatData = {
        {"enable", "true"},
        {"ioat_cnt_numa0", "8"},
        {"ioat_cnt_numa1", "8"}
    };
    vector<ConfigKeyValue> affinityManagerData = {
        {"use_config", "true"},
        {"reactor", "\"0\""},
        {"udd_io_worker", "\"1\""},
        {"event_scheduler", "\"2\""},
        {"event_worker", "\"3-5\""},
        {"general_usage", "\"6\""},
    #if defined QOS_ENABLED_BE
        {"qos", "\"7\""},
    #endif
        {"meta_scheduler", "\"8\""},
        {"meta_io", "\"9-10\""}
    };
    vector<ConfigKeyValue> userNvmeDriverData = {
        {"use_config", "true"},
        {"ssd_timeout_us", "8000000"},
        {"retry_count_backend_io", "5"},
        {"retry_count_frontend_io", "3"}
    };
    vector<ConfigKeyValue> perfImpactData = {
        {"gc", "\"high\""},
        {"rebuild", "\"low\""}
    };

    using ConfigList =
        std::vector<ConfigModuleData>;
    ConfigList defaultConfig = {
        {"journal", journalData},
        {"logger", loggerData},
        {"ioat", ioatData},
        {"affinity_manager", affinityManagerData},
        {"user_nvme_driver", userNvmeDriverData},
        {"perf_impact", perfImpactData}
    };

    const string CONFIGURATION_PATH = "/etc/pos/";
    const string CONFIGURATION_NAME = "pos.conf";
    const string DEFAULT_CONFIGURATION_NAME = "default_pos.conf";
};

} // namespace pos

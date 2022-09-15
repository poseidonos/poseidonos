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
    ConfigurationDir(void)
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
        {"enable", "true"},
        {"buffer_size_in_mb", "0"},
        {"number_of_log_groups", "2"},
        {"debug_mode", "false"},
        {"interval_in_msec_for_metric", "1000"}
    };
    vector<ConfigKeyValue> flushData = {
        {"enable", "false"},
        {"internal_flush_enable", "true"},
        {"internal_flush_threshold", "5"}
    };
    vector<ConfigKeyValue> adminData = {
        {"smart_log_page", "false"}
    };
    vector<ConfigKeyValue> loggerData = {
        {"logfile_size_in_mb", "50"},
        {"logfile_rotation_count", "20"},
        {"min_allowable_log_level", "\"info\""},
        // TODO (mj): The default value of structured_logging will be
        // TRUE after implementing the strcutured logging functionality.
        {"enable_structured_logging", "false"},
    };
    vector<ConfigKeyValue> eventSchedulerData = {
        {"numa_dedicated", "false"}
    };
    vector<ConfigKeyValue> debugData = {
        {"memory_checker", "false"}
    };
    vector<ConfigKeyValue> ioatData = {
        {"enable", "true"}
    };
    vector<ConfigKeyValue> affinityManagerData = {
        {"use_config", "true"},
        {"reactor", "\"0\""},
        {"udd_io_worker", "\"1\""},
        {"event_scheduler", "\"2\""},
        {"event_worker", "\"3-5\""},
        {"general_usage", "\"6\""},
        {"qos", "\"7\""},
        {"meta_scheduler", "\"8\""},
        {"meta_io", "\"9-10\""},
        {"air", "\"11\""}
    };
    vector<ConfigKeyValue> userNvmeDriverData = {
        {"use_config", "true"},
        {"ssd_timeout_us", "8000000"},
        {"retry_count_backend_io", "5"},
        {"retry_count_frontend_io", "3"}
    };
    vector<ConfigKeyValue> perfImpactData = {
        {"rebuild", "\"highest\""}
    };
    vector<ConfigKeyValue> feQosData = {
        {"enable", "false"}
    };
    vector<ConfigKeyValue> flowControlData = {
        {"enable", "true"},
        {"use_default", "true"},
        {"refill_timeout_in_msec", "1000"},
        {"total_token_in_stripe", "1024"},
        {"strategy", "\"linear\""},
        {"flow_control_target_percent", "35"},
        {"flow_control_urgent_percent", "15"},
        {"flow_control_target_segment", "10"},
        {"flow_control_urgent_segment", "5"}
    };
    vector<ConfigKeyValue> transportData = {
        {"enable", "false"},
        {"type", "\"tcp\""},
        {"buf_cache_size", "64"},
        {"num_shared_buffer", "4096"}
    };
    vector<ConfigKeyValue> metaFsData = {
        {"mio_pool_capacity", "32768"},
        {"mpio_pool_capacity", "32768"},
        {"write_mpio_cache_capacity", "32"},
        {"direct_access_for_journal_enable", "true"},
        {"time_interval_in_milliseconds_for_metric", "1000"},
        {"sampling_skip_count", "100"},
        {"wrr_count_special_purpose_map", "1"},
        {"wrr_count_journal", "1"},
        {"wrr_count_map", "1"},
        {"wrr_count_general", "1"},
    };
    vector<ConfigKeyValue> wtData = {
        {"enable", "false"}
    };
    vector<ConfigKeyValue> traceData = {
        {"enable", "false"},
        {"collector_endpoint", "\"http://localhost:3418/v1/traces\""}
    };
    vector<ConfigKeyValue> rebuildData = {
        {"auto_start", "true"}
    };

    using ConfigList =
        std::vector<ConfigModuleData>;
    ConfigList defaultConfig = {
        {"journal", journalData},
        {"flush", flushData},
        {"admin", adminData},
        {"logger", loggerData},
        {"event_scheduler", eventSchedulerData},
        {"debug", debugData},
        {"ioat", ioatData},
        {"affinity_manager", affinityManagerData},
        {"user_nvme_driver", userNvmeDriverData},
        {"perf_impact", perfImpactData},
        {"fe_qos", feQosData},
        {"flow_control", flowControlData},
        {"transport", transportData},
        {"metafs", metaFsData},
        {"write_through", wtData},
        {"trace", traceData},
        {"rebuild", rebuildData}
    };

    const string CONFIGURATION_PATH = "/etc/pos/";
    const string CONFIGURATION_NAME = "pos.conf";
    const string DEFAULT_CONFIGURATION_NAME = "default_pos.conf";
};

} // namespace pos

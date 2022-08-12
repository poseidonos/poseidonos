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

#include "src/telemetry/telemetry_config/telemetry_config.h"

#include <yaml-cpp/yaml.h>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdio>

#include "src/helper/file/directory.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
TelemetryConfig::TelemetryConfig(const std::string& path, const std::string& fileName)
{
    readers.insert({ConfigPriority::Priority_1st, std::make_shared<CliConfigReader>()});
    readers.insert({ConfigPriority::Priority_2nd, std::make_shared<EnvVariableConfigReader>()});
    readers.insert({ConfigPriority::Priority_3rd, std::make_shared<FileConfigReader>()});

    // for test
    std::string fileFullPath = path + fileName;

    if (!path.compare("") && !fileName.compare(""))
    {
        DefaultConfiguration conf;
        fileFullPath = conf.ConfigurationDir() + ConfigurationFileName();
    }

    std::shared_ptr<FileConfigReader> fileReader = std::dynamic_pointer_cast<FileConfigReader>(readers[ConfigPriority::Priority_3rd]);
    if ((nullptr != fileReader) && (false == fileReader->Init(fileFullPath)))
    {
        POS_TRACE_ERROR(EID(TELEMETRY_CONFIG_BAD_FILE),
            "Poseidon OS will stop by invalid telemetry config.");
        POS_TRACE_ERROR(EID(TELEMETRY_CONFIG_BAD_FILE),
            "Please type \"make install\" before running pos.");

        assert(false);
    }
}

TelemetryConfig::~TelemetryConfig(void)
{
    readers.clear();
    observers.clear();
}

bool
TelemetryConfig::Register(const TelemetryConfigType type, const std::string& key, std::shared_ptr<ConfigObserver> observer)
{
    const std::string newKey = GetCompositeKey(type, key);

    if (_Find(newKey, observer))
        return false;

    observers.insert({newKey, observer});

    return true;
}

void
TelemetryConfig::RequestToNotify(const TelemetryConfigType type, const std::string& key, const std::string& value)
{
    const std::string newKey = GetCompositeKey(type, key);

    for (auto it = observers.lower_bound(newKey); it != observers.upper_bound(newKey); ++it)
        it->second->Notify(key, value);
}

ClientConfig&
TelemetryConfig::GetClient(void)
{
    std::shared_ptr<FileConfigReader> fileReader =
        std::dynamic_pointer_cast<FileConfigReader>(readers[ConfigPriority::Priority_3rd]);

    return fileReader->GetClient();
}

const std::string
TelemetryConfig::ConfigurationFileName(void) const
{
    return TelemetryConfig::CONFIGURATION_NAME;
}

const std::string
TelemetryConfig::GetCompositeKey(const TelemetryConfigType type, const std::string& key)
{
    return (to_string(type) + "|" + key);
}

bool
TelemetryConfig::_Find(const std::string& key, const std::shared_ptr<ConfigObserver> observer)
{
    for (auto it = observers.lower_bound(key); it != observers.upper_bound(key); ++it)
    {
        if (it->second == observer)
        {
            return true;
        }
    }

    return false;
}
} // namespace pos

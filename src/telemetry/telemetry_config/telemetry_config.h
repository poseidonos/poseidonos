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

#include <map>
#include <string>

#include "src/lib/singleton.h"
#include "src/master_context/default_configuration.h"
#include "src/telemetry/common/config_observer.h"
#include "src/telemetry/common/config_reader.h"
#include "src/telemetry/telemetry_config/cli_config_reader.h"
#include "src/telemetry/telemetry_config/env_variable_config_reader.h"
#include "src/telemetry/telemetry_config/file_config_reader.h"

namespace pos
{
class TelemetryConfig
{
public:
    TelemetryConfig(std::string path = "", std::string fileName = "");
    virtual ~TelemetryConfig(void);

    virtual bool Register(std::string key, ConfigObserver* observer);

    virtual ClientConfig& GetClient(void)
    {
        return fileReader->GetClient();
    }

    virtual ServerConfig& GetServer(void)
    {
        return fileReader->GetServer();
    }

    virtual std::string ConfigurationDir(void)
    {
        DefaultConfiguration conf;
        return conf.ConfigurationDir();
    }

    virtual std::string ConfigurationFileName(void)
    {
        return TelemetryConfig::CONFIGURATION_NAME;
    }

    virtual void CreateFile(std::string path, std::string fileName);
    virtual void RemoveFile(std::string path, std::string fileName);

    // only for test
    std::map<ConfigPriority, ConfigReader*>& GetReadersMap(void)
    {
        return readers;
    }

    // only for test
    std::multimap<std::string, ConfigObserver*>& GetObserversMap(void)
    {
        return observers;
    }

private:
    bool _Find(std::string key, ConfigObserver* observer);

    CliConfigReader* cliReader = nullptr;
    EnvVariableConfigReader* envReader = nullptr;
    FileConfigReader* fileReader = nullptr;

    std::string defaultConfiguration = "";
    const std::string CONFIGURATION_NAME = "telemetry_default.yaml";

    // priority to reader
    std::map<ConfigPriority, ConfigReader*> readers;
    // key to observer
    std::multimap<std::string, ConfigObserver*> observers;
};

using TelemetryConfigSingleton = Singleton<TelemetryConfig>;
} // namespace pos

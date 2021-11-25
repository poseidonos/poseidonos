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

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdio>

#include "src/helper/file/directory.h"
#include "src/helper/file/file.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
TelemetryConfig::TelemetryConfig(std::string path, std::string fileName)
{
    // make default telemetry config
    defaultConfiguration = "telemetry:\n"
                    "    client:\n"
                    "        target:\n"
                    "            ip: localhost\n"
                    "            port: 10101\n"
                    "        enabled: true\n"
                    "        rate_limit: 60\n"
                    "        timeout_sec: 1\n"
                    "        circuit_break_policy: none\n"
                    "    server:\n"
                    "        ip: localhost\n"
                    "        port: 10101\n"
                    "        enabled: true\n"
                    "        buffer_size:\n"
                    "            counters: 10000\n"
                    "            histograms: 10000\n"
                    "            gauges: 10000\n"
                    "            latencies: 10000\n"
                    "            typed_objects: 10000\n"
                    "            influxdb_rows: 10000";

    cliReader = new CliConfigReader();
    envReader = new EnvVariableConfigReader();
    fileReader = new FileConfigReader();

    readers.insert({ConfigPriority::Priority_1st, cliReader});
    readers.insert({ConfigPriority::Priority_2nd, envReader});
    readers.insert({ConfigPriority::Priority_3rd, fileReader});

    std::string fileFullPath = path + fileName;

    if (!path.compare("") && !fileName.compare(""))
    {
        CreateFile(ConfiguratinDir(), ConfigurationFileName());
        fileFullPath = ConfiguratinDir() + ConfigurationFileName();
    }

    if (!fileReader->Init(fileFullPath))
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::TELEMETRY_ERROR_MSG,
                    "Default Telemetry Config will be used.");

        RemoveFile(ConfiguratinDir(), ConfigurationFileName());
        CreateFile(ConfiguratinDir(), ConfigurationFileName());
    }
}

TelemetryConfig::~TelemetryConfig(void)
{
    for (auto it = readers.begin(); it != readers.end(); it++)
    {
        delete it->second;
    }
    readers.clear();
    observers.clear();
}

bool
TelemetryConfig::Register(std::string key, ConfigObserver* observer)
{
    if (true == _Find(key, observer))
    {
        return false;
    }

    observers.insert({key, observer});

    return true;
}

void
TelemetryConfig::CreateFile(std::string path, std::string fileName)
{
    string filePath = path + fileName;

    if (false == DirExists(path))
        MakeDir(path);

    if (true == FileExists(filePath))
        return;

    std::ofstream outfile(filePath.data());

    outfile << defaultConfiguration << std::endl;
    outfile.close();
}

void
TelemetryConfig::RemoveFile(std::string path, std::string fileName)
{
    string filePath = path + fileName;

    if (false == FileExists(filePath))
        return;

    remove(filePath.c_str());
}

bool
TelemetryConfig::_Find(std::string key, ConfigObserver* observer)
{
    std::multimap<std::string, ConfigObserver*>& mm = observers;

    for (auto it = mm.lower_bound(key); it != mm.upper_bound(key); ++it)
    {
        if (it->second == observer)
        {
            return true;
        }
    }

    return false;
}
} // namespace pos

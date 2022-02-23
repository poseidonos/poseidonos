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
#include "src/helper/file/file.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
TelemetryConfig::TelemetryConfig(std::string path, std::string fileName)
{
    cliReader = new CliConfigReader();
    envReader = new EnvVariableConfigReader();
    fileReader = new FileConfigReader();

    readers.insert({ConfigPriority::Priority_1st, cliReader});
    readers.insert({ConfigPriority::Priority_2nd, envReader});
    readers.insert({ConfigPriority::Priority_3rd, fileReader});

    std::string fileFullPath = path + fileName;

    if (!path.compare("") && !fileName.compare(""))
    {
        fileFullPath = ConfigurationDir() + ConfigurationFileName();
    }

    if (!fileReader->Init(fileFullPath))
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::TELEMETRY_CONFIG_BAD_FILE,
            "Poseidon OS will stop by invalid telemetry config.");
        POS_TRACE_ERROR((int)POS_EVENT_ID::TELEMETRY_CONFIG_BAD_FILE,
            "Please type \"make install\" before running pos.");

        assert(false);
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
TelemetryConfig::Register(TelemetryConfigType type, std::string key, ConfigObserver* observer)
{
    std::string newKey = GetKey(type, key);

    if (true == _Find(newKey, observer))
    {
        return false;
    }

    observers.insert({newKey, observer});

    return true;
}

bool
TelemetryConfig::RequestToNotify(TelemetryConfigType type, std::string key, std::string value)
{
    std::string newKey = GetKey(type, key);
    bool existed = false;

    for (auto it = observers.lower_bound(newKey); it != observers.upper_bound(newKey); ++it)
    {
        it->second->Notify(key, value);
        existed = true;
    }

    return existed;
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

std::string
TelemetryConfig::GetKey(TelemetryConfigType type, std::string key)
{
    return (to_string(type) + "|" + key);
}

bool
TelemetryConfig::_Find(std::string key, ConfigObserver* observer)
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

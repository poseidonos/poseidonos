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

#include "src/telemetry/telemetry_config/telemetry_config.h"

#include <yaml-cpp/yaml.h>

#include <map>
#include <string>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
TelemetryConfig::TelemetryConfig(void)
: cliReader(nullptr),
  envReader(nullptr),
  fileReader(nullptr)
{
}

TelemetryConfig::TelemetryConfig(std::string fileName)
: TelemetryConfig()
{
    cliReader = new CliConfigReader();
    envReader = new EnvVariableConfigReader();
    fileReader = new FileConfigReader();

    fileReader->Init(fileName);

    readers.insert({ConfigPriority::Priority_1st, cliReader});
    readers.insert({ConfigPriority::Priority_2nd, envReader});
    readers.insert({ConfigPriority::Priority_3rd, fileReader});
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

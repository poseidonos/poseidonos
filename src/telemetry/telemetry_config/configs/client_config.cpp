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

#include "src/telemetry/telemetry_config/configs/client_config.h"

#include <string>
#include <unordered_map>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
ClientConfig::ClientConfig(void)
{
    values.clear();
}

ClientConfig::~ClientConfig(void)
{
}

bool
ClientConfig::Init(YAML::Node& node)
{
    try
    {
        std::string key = "ip";
        std::string value = node["target"][key].as<std::string>();
        target.UpdateConfig(TelemetryConfigType::Client_Target, key, value);

        key = "port";
        value = node["target"][key].as<std::string>();
        target.UpdateConfig(TelemetryConfigType::Client_Target, key, value);

        key = "enabled";
        value = node[key].as<std::string>();
        UpdateConfig(type, key, value);

        key = "rate_limit";
        value = node[key].as<std::string>();
        UpdateConfig(type, key, value);

        key = "timeout_sec";
        value = node[key].as<std::string>();
        UpdateConfig(type, key, value);

        key = "circuit_break_policy";
        value = node[key].as<std::string>();
        UpdateConfig(type, key, value);
    }
    catch (YAML::BadConversion& e)
    {
        POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG),
            "{}", e.msg);

        return false;
    }

    return true;
}

uint64_t
ClientConfig::GetRateLimit(void)
{
    std::string value = _GetValue("rate_limit");

    return (value == DEFAULT_YAML_VALUE) ? 0 : std::stoi(value);
}

uint64_t
ClientConfig::GetTimeoutSec(void)
{
    std::string value = _GetValue("timeout_sec");

    return (value == DEFAULT_YAML_VALUE) ? 0 : std::stoi(value);
}

std::string
ClientConfig::GetCircuitBreakPolicy(void)
{
    return _GetValue("circuit_break_policy");
}

bool
ClientConfig::IsEnabled(void)
{
    std::string value = _GetValue("enabled");

    return (value == DEFAULT_YAML_VALUE || !value.compare("false")) ? false : true;
}

TargetConfig&
ClientConfig::GetTarget(void)
{
    return target;
}
} // namespace pos

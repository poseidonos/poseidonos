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

#include "src/telemetry/telemetry_config/configs/server_config.h"

#include <string>
#include <unordered_map>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
ServerConfig::ServerConfig(void)
{
    values.clear();
}

ServerConfig::~ServerConfig(void)
{
}

bool
ServerConfig::Init(YAML::Node& node)
{
    try
    {
        std::string key = "counters";
        std::string value = node["buffer_size"][key].as<std::string>();
        bufferSize.UpdateConfig(TelemetryConfigType::Server_BufferSize, key, value);

        key = "histograms";
        value = node["buffer_size"][key].as<std::string>();
        bufferSize.UpdateConfig(TelemetryConfigType::Server_BufferSize, key, value);

        key = "gauges";
        value = node["buffer_size"][key].as<std::string>();
        bufferSize.UpdateConfig(TelemetryConfigType::Server_BufferSize, key, value);

        key = "latencies";
        value = node["buffer_size"][key].as<std::string>();
        bufferSize.UpdateConfig(TelemetryConfigType::Server_BufferSize, key, value);

        key = "typed_objects";
        value = node["buffer_size"][key].as<std::string>();
        bufferSize.UpdateConfig(TelemetryConfigType::Server_BufferSize, key, value);

        key = "influxdb_rows";
        value = node["buffer_size"][key].as<std::string>();
        bufferSize.UpdateConfig(TelemetryConfigType::Server_BufferSize, key, value);

        key = "ip";
        value = node[key].as<std::string>();
        UpdateConfig(type, key, value);

        key = "port";
        value = node[key].as<std::string>();
        UpdateConfig(type, key, value);

        key = "enabled";
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

std::string
ServerConfig::GetIp(void)
{
    return _GetValue("ip");
}

uint64_t
ServerConfig::GetPort(void)
{
    std::string value = _GetValue("port");

    return (value == DEFAULT_YAML_VALUE) ? 0 : std::stoi(value);
}

bool
ServerConfig::IsEnabled(void)
{
    std::string value = _GetValue("enabled");

    return (value == DEFAULT_YAML_VALUE || !value.compare("false")) ? false : true;
}

BufferSizeConfig&
ServerConfig::GetBufferSize(void)
{
    return bufferSize;
}
} // namespace pos

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
#include "src/network/transport_configuration.h"

#include <algorithm>
#include <string>

#include "src/include/nvmf_const.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

namespace pos
{
TransportConfiguration::TransportConfiguration(ConfigManager* configManager)
: TransportConfiguration(configManager, nullptr)
{
}

TransportConfiguration::TransportConfiguration(ConfigManager* configManager, SpdkRpcClient* inputRpcClient)
: configManager(configManager),
  trtype(DEFAULT_TRANSPORT_TYPE),
  bufCacheSize(DEFAULT_BUF_CACHE_SIZE),
  numSharedBuf(DEFAULT_NUM_SHARED_BUF),
  ioUnitSize(DEFAULT_IO_UNIT_SIZE),
  rpcClient(inputRpcClient)
{
    if (nullptr == rpcClient)
    {
        rpcClient = new SpdkRpcClient();
    }
}

TransportConfiguration::~TransportConfiguration(void)
{
    if (nullptr != rpcClient)
    {
        delete rpcClient;
    }
}

void
TransportConfiguration::ReadConfig(void)
{
    int ret = configManager->GetValue("transport", "type", &trtype, ConfigType::CONFIG_TYPE_STRING);
    if (EID(SUCCESS) == ret)
    {
        ret = configManager->GetValue("transport", "buf_cache_size", &bufCacheSize, ConfigType::CONFIG_TYPE_UINT32);
        if (EID(SUCCESS) != ret)
        {
            POS_EVENT_ID eventId = EID(IONVMF_FAIL_TO_READ_TRANSPORT_CONFIG);
            POS_TRACE_WARN(static_cast<uint32_t>(eventId), "Fail to read transport config. Default buf_cache_size: {}", bufCacheSize);
        }
        ret = configManager->GetValue("transport", "num_shared_buffer", &numSharedBuf, ConfigType::CONFIG_TYPE_UINT32);
        if (EID(SUCCESS) != ret)
        {
            POS_EVENT_ID eventId = EID(IONVMF_FAIL_TO_READ_TRANSPORT_CONFIG);
            POS_TRACE_WARN(static_cast<uint32_t>(eventId),
                "Fail to read transport config. Default num_shared_buffer: {} (May change according to the env.)", numSharedBuf);
        }
    }
    else
    {
        POS_EVENT_ID eventId = EID(IONVMF_FAIL_TO_READ_TRANSPORT_CONFIG);
        POS_TRACE_WARN(static_cast<uint32_t>(eventId), "Fail to read transport config. Default transport type: {}", trtype);
    }
}

void
TransportConfiguration::CreateTransport(void)
{
    if (false == _IsEnabled())
    {
        return;
    }

    ReadConfig();

    std::transform(trtype.begin(), trtype.end(), trtype.begin(), ::tolower);
    auto result = rpcClient->TransportCreate(trtype, bufCacheSize, numSharedBuf, ioUnitSize);
    if (result.first != 0)
    {
        POS_EVENT_ID eventId = EID(IONVMF_FAIL_TO_CREATE_TRANSPORT);
        POS_TRACE_ERROR(static_cast<uint32_t>(eventId), "Fail to create transport : {}", result.second);
    }
}

bool
TransportConfiguration::_IsEnabled(void)
{
    bool enabled = false;
    int ret = configManager->GetValue("transport", "enable", &enabled, ConfigType::CONFIG_TYPE_BOOL);
    if (EID(SUCCESS) != ret)
    {
        POS_EVENT_ID eventId = EID(IONVMF_FAIL_TO_READ_TRANSPORT_CONFIG);
        POS_TRACE_WARN(static_cast<uint32_t>(eventId), "Fail to read transport config. Need to create tranport manually.");
        return false;
    }
    return enabled;
}
} // namespace pos

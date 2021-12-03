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

#include <gtest/gtest.h>

#include <string>

#include "yaml-cpp/yaml.h"

namespace pos
{
TEST(ClientConfig, Create_testIfClientConfigCanBeCreated)
{
    ClientConfig* client = new ClientConfig();
    delete client;
}

TEST(ClientConfig, Update_testIfClientConfigCanContainOwnValues)
{
    ClientConfig client;

    std::string key = "rate_limit";
    uint64_t value_int = 10;
    client.UpdateConfig(TelemetryConfigType::Client, key, value_int);

    key = "timeout_sec";
    value_int = 1;
    client.UpdateConfig(TelemetryConfigType::Client, key, value_int);

    key = "circuit_break_policy";
    std::string value_str = "circuit_break_policy";
    client.UpdateConfig(TelemetryConfigType::Client, key, value_str);

    key = "enabled";
    client.UpdateConfig(TelemetryConfigType::Client, key, true);

    EXPECT_EQ(client.GetRateLimit(), 10);
    EXPECT_EQ(client.GetTimeoutSec(), 1);
    EXPECT_EQ(client.GetCircuitBreakPolicy(), "circuit_break_policy");
    EXPECT_EQ(client.IsEnabled(), true);
}

TEST(ClientConfig, Update_testIfClientConfigCanUpdateValue)
{
    ClientConfig client;

    std::string key = "enabled";
    client.UpdateConfig(TelemetryConfigType::Client, key, true);

    EXPECT_EQ(client.IsEnabled(), true);

    client.UpdateConfig(TelemetryConfigType::Client, key, false);

    EXPECT_EQ(client.IsEnabled(), false);
}
} // namespace pos

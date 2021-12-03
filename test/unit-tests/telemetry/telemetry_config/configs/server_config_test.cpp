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

#include <gtest/gtest.h>

#include <string>

#include "yaml-cpp/yaml.h"

namespace pos
{
TEST(ServerConfig, Create_testIfServerConfigCanBeCreated)
{
    ServerConfig* server = new ServerConfig();
    delete server;
}

TEST(ServerConfig, Update_testIfServerConfigCanContainOwnValues)
{
    ServerConfig server;

    std::string key_1 = "ip";
    std::string value_1 = "10.10.10.10";
    server.UpdateConfig(TelemetryConfigType::Server, key_1, value_1);

    std::string key_2 = "port";
    uint64_t value_2 = 1234;
    server.UpdateConfig(TelemetryConfigType::Server, key_2, value_2);

    std::string key_3 = "enabled";
    bool value_3 = true;
    server.UpdateConfig(TelemetryConfigType::Server, key_3, value_3);

    EXPECT_EQ(server.GetIp(), value_1);
    EXPECT_EQ(server.GetPort(), value_2);
    EXPECT_EQ(server.IsEnabled(), value_3);
}

TEST(ServerConfig, Update_testIfClientConfigCanUpdateValue)
{
    ServerConfig server;

    std::string key = "enabled";
    server.UpdateConfig(TelemetryConfigType::Server, key, true);

    EXPECT_EQ(server.IsEnabled(), true);

    server.UpdateConfig(TelemetryConfigType::Server, key, false);

    EXPECT_EQ(server.IsEnabled(), false);
}
} // namespace pos

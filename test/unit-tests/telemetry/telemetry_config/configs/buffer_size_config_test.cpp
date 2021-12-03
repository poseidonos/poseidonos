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

#include "src/telemetry/telemetry_config/configs/buffer_size_config.h"

#include <gtest/gtest.h>

#include <string>

#include "yaml-cpp/yaml.h"

namespace pos
{
TEST(BufferSizeConfig, Create_testIfBufferSizeConfigCanBeCreated)
{
    BufferSizeConfig* bufferSize = new BufferSizeConfig();
    delete bufferSize;
}

TEST(BufferSizeConfig, Update_testIfBufferSizeConfigCanContainOwnValues)
{
    BufferSizeConfig obj;

    std::string key = "counters";
    uint64_t value = 10000;
    obj.UpdateConfig(TelemetryConfigType::Server_BufferSize, key, value);

    key = "gauges";
    obj.UpdateConfig(TelemetryConfigType::Server_BufferSize, key, value);

    key = "histograms";
    obj.UpdateConfig(TelemetryConfigType::Server_BufferSize, key, value);

    key = "influxdb_rows";
    obj.UpdateConfig(TelemetryConfigType::Server_BufferSize, key, value);

    key = "latencies";
    obj.UpdateConfig(TelemetryConfigType::Server_BufferSize, key, value);

    key = "typed_objects";
    obj.UpdateConfig(TelemetryConfigType::Server_BufferSize, key, value);

    EXPECT_EQ(obj.GetCounters(), value);
    EXPECT_EQ(obj.GetGauges(), value);
    EXPECT_EQ(obj.GetHistograms(), value);
    EXPECT_EQ(obj.GetInfluxdb_rows(), value);
    EXPECT_EQ(obj.GetLatencies(), value);
    EXPECT_EQ(obj.GetTyped_objects(), value);
}
} // namespace pos

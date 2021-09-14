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

#include "src/telemetry/telemetry_config/buffer_size.h"

#include <gtest/gtest.h>

#include <string>

#include "yaml-cpp/yaml.h"

namespace pos
{
TEST(BufferSize, Creation)
{
    BufferSize* bufferSize = new BufferSize();
    delete bufferSize;
}

TEST(BufferSize, CheckDefaultMethod)
{
    BufferSize obj;

    std::string key = "counters";
    std::string value = "counters";
    obj.UpdateConfig(key, value);

    key = "gauges";
    value = "gauges";
    obj.UpdateConfig(key, value);

    key = "histograms";
    value = "histograms";
    obj.UpdateConfig(key, value);

    key = "influxdb_rows";
    value = "influxdb_rows";
    obj.UpdateConfig(key, value);

    key = "latencies";
    value = "latencies";
    obj.UpdateConfig(key, value);

    key = "typed_objects";
    value = "typed_objects";
    obj.UpdateConfig(key, value);

    EXPECT_EQ(obj.GetCounters(), "counters");
    EXPECT_EQ(obj.GetGauges(), "gauges");
    EXPECT_EQ(obj.GetHistograms(), "histograms");
    EXPECT_EQ(obj.GetInfluxdb_rows(), "influxdb_rows");
    EXPECT_EQ(obj.GetLatencies(), "latencies");
    EXPECT_EQ(obj.GetTyped_objects(), "typed_objects");
}
} // namespace pos

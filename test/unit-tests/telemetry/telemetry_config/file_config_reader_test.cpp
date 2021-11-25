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

#include "src/telemetry/telemetry_config/file_config_reader.h"

#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "yaml-cpp/node/impl.h"
#include "yaml-cpp/yaml.h"

namespace pos
{
TEST(FileConfigReader, LoadFile_Positive)
{
    std::string fileName = "./unit-tests/telemetry/telemetry_config/test.yaml";

    FileConfigReader* reader = new FileConfigReader();

    EXPECT_EQ(reader->Init(fileName), true);

    delete reader;
}

TEST(FileConfigReader, LoadFile_Negative)
{
    std::string fileName = "";

    FileConfigReader* reader = new FileConfigReader();

    EXPECT_EQ(reader->Init(fileName), false);

    delete reader;
}

TEST(FileConfigReader, LoadClientValue_Positive)
{
    std::string fileName = "./unit-tests/telemetry/telemetry_config/test.yaml";
    FileConfigReader* reader = new FileConfigReader();

    EXPECT_EQ(reader->Init(fileName), true);

    EXPECT_EQ(reader->GetClient().GetTarget().GetIp(), "localhost");
    EXPECT_EQ(reader->GetClient().GetTarget().GetPort(), 10101);
    EXPECT_EQ(reader->GetClient().GetRateLimit(), 60);

    delete reader;
}

TEST(FileConfigReader, LoadServerValue_Positive)
{
    std::string fileName = "./unit-tests/telemetry/telemetry_config/test.yaml";
    FileConfigReader* reader = new FileConfigReader();

    EXPECT_EQ(reader->Init(fileName), true);

    EXPECT_EQ(reader->GetServer().GetIp(), "localhost");
    EXPECT_EQ(reader->GetServer().GetPort(), 10101);
    EXPECT_EQ(reader->GetServer().GetBufferSize().GetCounters(), 10000);
    EXPECT_EQ(reader->GetServer().GetBufferSize().GetGauges(), 10000);

    delete reader;
}

TEST(FileConfigReader, LoadValue_Negative)
{
    std::string fileName = ".";
    FileConfigReader* reader = new FileConfigReader();

    EXPECT_THROW(reader->Init(fileName), std::runtime_error);

    delete reader;
}
} // namespace pos

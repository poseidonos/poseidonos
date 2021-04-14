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

#include "gtest/gtest.h"

#include "src/include/memory.h"
#include "src/logger/logger.h"

#include "test/integration-tests/journal/utils/journal_configuration_builder.h"

namespace pos
{
class JournalConfigurationTest : public ::testing::Test
{
public:
    JournalConfigurationTest(void);
    virtual ~JournalConfigurationTest(void) {}
    void TestConfigureLogBufferSize(uint64_t input, uint64_t expected);
    uint64_t GetExpectedValue(uint64_t input);

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    TestInfo testInfo;
};

JournalConfigurationTest::JournalConfigurationTest(void)
{
}

void
JournalConfigurationTest::SetUp(void)
{
}

void
JournalConfigurationTest::TearDown(void)
{
}

void
JournalConfigurationTest::TestConfigureLogBufferSize(uint64_t inputBufferSize, uint64_t expected)
{
    JournalConfigurationBuilder builder(&testInfo);
    builder.SetJournalEnable(true)
        ->SetLogBufferSize(inputBufferSize);

    JournalConfiguration* config = builder.Build();

    config->Init();

    EXPECT_TRUE(config->GetLogBufferSize() == expected);
    EXPECT_TRUE(config->GetLogBufferSize() % testInfo.metaPageSize == 0);

    delete config;
}

uint64_t
JournalConfigurationTest::GetExpectedValue(uint64_t input)
{
    // 2 is number of log groups
    // TODO (cheolho.kang) change to get numLogGroups from JournalConfiguration
    return AlignDown(input - testInfo.metaPageSize, testInfo.metaPageSize * 2);
}

TEST_F(JournalConfigurationTest, ConfigureLogBufferSize)
{
    POS_TRACE_DEBUG(9999, "JournalConfigurationTest::ConfigureLogBufferSize");

    uint64_t logBufferSize = 0;
    uint64_t expectedLogBufferSize = 0;

    // Test default log buffer size
    logBufferSize = 0;
    expectedLogBufferSize = GetExpectedValue(testInfo.metaPartitionSize);
    TestConfigureLogBufferSize(logBufferSize, expectedLogBufferSize);

    // Test meta-page aligned log buffer size
    logBufferSize = AlignDown(testInfo.metaPartitionSize, testInfo.metaPageSize);
    expectedLogBufferSize = GetExpectedValue(logBufferSize);
    TestConfigureLogBufferSize(logBufferSize, expectedLogBufferSize);

    // Test meta-page un-aligned log buffer size
    logBufferSize = AlignDown(testInfo.metaPartitionSize / 2, testInfo.metaPageSize) + 1;
    expectedLogBufferSize = GetExpectedValue(logBufferSize);
    TestConfigureLogBufferSize(logBufferSize, expectedLogBufferSize);

    // Test log buffer size bigger than partition size
    logBufferSize = testInfo.metaPartitionSize * 2;
    expectedLogBufferSize = GetExpectedValue(testInfo.metaPartitionSize);
    TestConfigureLogBufferSize(logBufferSize, expectedLogBufferSize);

    // Test NVRAM doesn't have free space to create new log buffer
    testInfo.metaPartitionSize = 0;
    logBufferSize = testInfo.metaPageSize * 10;
    expectedLogBufferSize = 0;
    TestConfigureLogBufferSize(logBufferSize, expectedLogBufferSize);
}
} // namespace pos

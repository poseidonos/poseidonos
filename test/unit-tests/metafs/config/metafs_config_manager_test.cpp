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

#include "src/metafs/config/metafs_config_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "test/unit-tests/master_context/config_manager_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
ACTION_P(SetArg2ToBoolAndReturn0, boolValue)
{
    *static_cast<bool*>(arg2) = boolValue;
    return 0;
}

ACTION_P(SetArg2ToLongAndReturn0, longValue)
{
    *static_cast<size_t*>(arg2) = longValue;
    return 0;
}

class MetaFsConfigManagerTest : public MetaFsConfigManager
{
public:
    MetaFsConfigManagerTest(void) = delete;
    MetaFsConfigManagerTest(ConfigManager* configManager)
    : MetaFsConfigManager(configManager)
    {
    }
    ~MetaFsConfigManagerTest(void)
    {
    }
    bool ValidateConfig(void) const
    {
        return MetaFsConfigManager::_ValidateConfig();
    }
};

class MetaFsConfigManagerFixture : public ::testing::Test
{
public:
    MetaFsConfigManagerFixture(void)
    : config(nullptr)
    {
    }
    ~MetaFsConfigManagerFixture(void)
    {
    }
    virtual void SetUp(void) override
    {
    }
    virtual void TearDown(void) override
    {
        delete manager;
        if (config != nullptr)
            delete config;
    }

protected:
    void _CreateConfigManager(const size_t mioPoolCapacity,
        const size_t mpioPoolCapacity, const bool writeMpioCacheEnabled,
        const size_t writeMpioCacheCapacity, const bool directAccessEnabled,
        const size_t timeIntervalInMillisecondsForMetric)
    {
        config = new NiceMock<MockConfigManager>;

        ON_CALL(*config, GetValue("metafs", "mio_pool_capacity", _, _))
            .WillByDefault(SetArg2ToLongAndReturn0(mioPoolCapacity));
        ON_CALL(*config, GetValue("metafs", "mpio_pool_capacity", _, _))
            .WillByDefault(SetArg2ToLongAndReturn0(mpioPoolCapacity));
        ON_CALL(*config, GetValue("metafs", "write_mpio_cache_enable", _, _))
            .WillByDefault(SetArg2ToBoolAndReturn0(writeMpioCacheEnabled));
        ON_CALL(*config, GetValue("metafs", "write_mpio_cache_capacity", _, _))
            .WillByDefault(SetArg2ToLongAndReturn0(writeMpioCacheCapacity));
        ON_CALL(*config, GetValue("metafs", "direct_access_for_journal_enable", _, _))
            .WillByDefault(SetArg2ToBoolAndReturn0(directAccessEnabled));
        ON_CALL(*config, GetValue("metafs", "time_interval_in_milliseconds_for_metric", _, _))
            .WillByDefault(SetArg2ToLongAndReturn0(timeIntervalInMillisecondsForMetric));

        manager = new MetaFsConfigManagerTest(config);
    }

    NiceMock<MockConfigManager>* config;
    MetaFsConfigManagerTest* manager;
};

TEST_F(MetaFsConfigManagerFixture, _ValidateConfig_testIfTheConfigIsInvalid)
{
    const size_t CAPACITY = 0;
    _CreateConfigManager(CAPACITY, CAPACITY, false, CAPACITY, false, 50);
    EXPECT_FALSE(manager->ValidateConfig());
}

TEST_F(MetaFsConfigManagerFixture, testIfTheMethodsReturnsExpectedValues)
{
    const size_t CAPACITY = 32;
    _CreateConfigManager(CAPACITY, CAPACITY + 1, true, CAPACITY + 2,
        true, CAPACITY + 3);
    manager->Init();

    EXPECT_EQ(manager->GetMioPoolCapacity(), CAPACITY);
    EXPECT_EQ(manager->GetMpioPoolCapacity(), CAPACITY + 1);
    EXPECT_TRUE(manager->IsWriteMpioCacheEnabled());
    EXPECT_EQ(manager->GetWriteMpioCacheCapacity(), CAPACITY + 2);
    EXPECT_TRUE(manager->IsDirectAccessEnabled());
    EXPECT_EQ(manager->GetTimeIntervalInMillisecondsForMetric(), CAPACITY + 3);
}

TEST_F(MetaFsConfigManagerFixture, testIfTheMethodsReturnsExpectedValues_Inverse)
{
    const size_t CAPACITY = 0;
    _CreateConfigManager(CAPACITY, CAPACITY, false, CAPACITY,
        false, CAPACITY);
    manager->Init();

    EXPECT_EQ(manager->GetMioPoolCapacity(), CAPACITY);
    EXPECT_EQ(manager->GetMpioPoolCapacity(), CAPACITY);
    EXPECT_FALSE(manager->IsWriteMpioCacheEnabled());
    EXPECT_EQ(manager->GetWriteMpioCacheCapacity(), CAPACITY);
    EXPECT_FALSE(manager->IsDirectAccessEnabled());
    EXPECT_EQ(manager->GetTimeIntervalInMillisecondsForMetric(), CAPACITY);
}
} // namespace pos

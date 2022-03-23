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

#include "src/journal_manager/config/journal_configuration.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/include/memory.h"
#include "src/include/pos_event_id.h"
#include "test/unit-tests/master_context/config_manager_mock.h"
#include "test/unit-tests/metafs/mai/metafs_file_control_api_mock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace pos
{
const uint64_t SIZE_MB = 1024 * 1024;

ACTION_P(SetArg2ToBoolAndReturn0, boolValue)
{
    *static_cast<bool*>(arg2) = boolValue;
    return 0;
}

ACTION_P(SetArg2ToLongAndReturn0, longValue)
{
    *static_cast<uint64_t*>(arg2) = longValue;
    return 0;
}

NiceMock<MockConfigManager>*
CreateMockConfigManager(bool isJournalEnabled, bool isDebugEnabled, uint64_t logBufferSizeInConfig)
{
    NiceMock<MockConfigManager>* configManager = new NiceMock<MockConfigManager>;

    ON_CALL(*configManager, GetValue("journal", "enable", _, _)).WillByDefault(SetArg2ToBoolAndReturn0(isJournalEnabled));
    ON_CALL(*configManager, GetValue("journal", "debug_mode", _, _)).WillByDefault(SetArg2ToBoolAndReturn0(isDebugEnabled));
    ON_CALL(*configManager, GetValue("journal", "buffer_size_in_mb", _, _)).WillByDefault(SetArg2ToLongAndReturn0(logBufferSizeInConfig));

    return configManager;
}

uint64_t
AlignDownWithMetaPage(uint64_t input, JournalConfiguration* config)
{
    return AlignDown(input - config->GetMetaPageSize(), config->GetMetaPageSize() * config->GetNumLogGroups());
}

TEST(JournalConfiguration, JournalConfiguration_testConstructedSuccessfully)
{
    // Given, When
    NiceMock<MockConfigManager> *configManager = CreateMockConfigManager(true, true, 0);
    JournalConfiguration config(configManager);

    // Then
    bool expected = true;
    EXPECT_EQ(config.IsEnabled(), expected);
    EXPECT_EQ(config.IsDebugEnabled(), expected);

    delete configManager;
}

TEST(JournalConfiguration, JournalConfiguration_testWithJournalDisabled)
{
    // Given
    NiceMock<MockConfigManager> configManager;

    // When: Journal is enabled
    EXPECT_CALL(configManager, GetValue("journal", "enable", _, _)).WillOnce([&](string module, string key, void* value, ConfigType type)
        {
            bool* targetToChange = static_cast<bool*>(value);
            *targetToChange = false;
            return 0;
        });

    JournalConfiguration config(&configManager);

    // Then: Will journal enable be false
    bool expected = false;
    EXPECT_EQ(config.IsEnabled(), expected);
}

TEST(JournalConfiguration, JournalConfiguration_testIfFailedToReadJournalEnabled)
{
    // Given
    NiceMock<MockConfigManager> configManager;

    // When: Failed to read journal enable value from config manager
    EXPECT_CALL(configManager, GetValue("journal", "enable", _, _)).WillOnce([&](string module, string key, void* value, ConfigType type)
        { return -1; });

    JournalConfiguration config(&configManager);

    // Then:  Will journal enable be false
    bool expected = false;
    EXPECT_EQ(config.IsEnabled(), expected);
}

TEST(JournalConfiguration, JournalConfiguration_testWhenFailedToReadLogBufferSize)
{
    // Given
    NiceMock<MockConfigManager> configManager;

    // When: Failed to read log buffer size from config manager
    EXPECT_CALL(configManager, GetValue);
    EXPECT_CALL(configManager, GetValue("journal", "enable", _, _)).WillOnce([&](string module, string key, void* value, ConfigType type)
        {
            bool* targetToChange = static_cast<bool*>(value);
            *targetToChange = true;
            return 0;
        });
    EXPECT_CALL(configManager, GetValue("journal", "buffer_size_in_mb", _, _)).WillOnce([&](string module, string key, void* value, ConfigType type)
        { return -1; });

    JournalConfiguration config(&configManager);

    // Then: Will log buffer size be default value
    uint64_t expected = 0;
    EXPECT_EQ(config.GetLogBufferSizeInConfig(), expected);
}

TEST(JournalConfiguration, Init_testIfMetaVolumeToUseIsUpdatedProperly)
{
    NiceMock<MockConfigManager> *configManager = CreateMockConfigManager(true, true, 0);
    JournalConfiguration config(configManager);

    config.Init(true);
    EXPECT_EQ(config.GetMetaVolumeToUse(), MetaVolumeType::JournalVolume);
    EXPECT_EQ(config.AreReplayWbStripesInUserArea(), true);

    config.Init(false);
    EXPECT_EQ(config.GetMetaVolumeToUse(), MetaVolumeType::NvRamVolume);
    EXPECT_EQ(config.AreReplayWbStripesInUserArea(), false);

    delete configManager;
}

TEST(JournalConfiguration, SetLogBufferSize_testIfLogBufferSetWhenLoadedLogBufferSizeIsNotZero)
{
    // Given: Loaded log buffer is is not zero such like dirty bringup
    NiceMock<MockMetaFsFileControlApi> metaFsCtrl;
    NiceMock<MockConfigManager> *configManager = CreateMockConfigManager(true, true, 0);
    JournalConfiguration config(configManager);

    uint64_t metaPageSize = 4032;
    uint64_t maxPartitionSize = 16 * SIZE_MB;

    ON_CALL(metaFsCtrl, EstimateAlignedFileIOSize).WillByDefault(Return(metaPageSize));
    ON_CALL(metaFsCtrl, GetAvailableSpace).WillByDefault(Return(maxPartitionSize));

    // When
    uint64_t loadedLogBufferSize = 16 * 1024;
    config.SetLogBufferSize(loadedLogBufferSize, &metaFsCtrl);

    // Then
    int numLogGroups = config.GetNumLogGroups();
    EXPECT_EQ(config.GetLogBufferSize(), loadedLogBufferSize);
    EXPECT_EQ(config.GetLogGroupSize(), loadedLogBufferSize / numLogGroups);
    EXPECT_EQ(config.GetMetaPageSize(), metaPageSize);

    delete configManager;
}

TEST(JournalConfiguration, SetLogBufferSize_testIfLogBufferSetWhenLoadedLogBufferSizeIsZero)
{
    // Given: Loaded log buffer is is zero such like clean bringup
    NiceMock<MockMetaFsFileControlApi> metaFsCtrl;
    uint64_t metaPageSize = 4032;
    uint64_t maxPartitionSize = 16 * SIZE_MB;
    uint64_t logBufferSizeInMB = maxPartitionSize / SIZE_MB / 2;

    NiceMock<MockConfigManager> *configManager = CreateMockConfigManager(true, true, logBufferSizeInMB);
    JournalConfiguration config(configManager);

    ON_CALL(metaFsCtrl, EstimateAlignedFileIOSize).WillByDefault(Return(metaPageSize));
    ON_CALL(metaFsCtrl, GetAvailableSpace).WillByDefault(Return(maxPartitionSize));

    // When
    config.SetLogBufferSize(0, &metaFsCtrl);

    // Then
    uint64_t expected = AlignDownWithMetaPage(logBufferSizeInMB * SIZE_MB, &config);
    int numLogGroups = config.GetNumLogGroups();
    EXPECT_EQ(config.GetLogBufferSize(), expected);
    EXPECT_EQ(config.GetLogGroupSize(), expected / numLogGroups);

    delete configManager;
}

TEST(JournalConfiguration, SetLogBufferSize_testIfLogBufferSetWhenLogBufferSizeIsDefaultValue)
{
    // Given: Log buffer size is zero
    NiceMock<MockMetaFsFileControlApi> metaFsCtrl;
    uint64_t metaPageSize = 4032;
    uint64_t maxPartitionSize = 16 * 1024 * 1024;

    uint64_t logBufferSizeInConfig = 0;
    NiceMock<MockConfigManager> *configManager = CreateMockConfigManager(true, true, logBufferSizeInConfig);
    JournalConfiguration config(configManager);

    ON_CALL(metaFsCtrl, EstimateAlignedFileIOSize).WillByDefault(Return(metaPageSize));
    ON_CALL(metaFsCtrl, GetAvailableSpace).WillByDefault(Return(maxPartitionSize));

    // When
    config.SetLogBufferSize(0, &metaFsCtrl);

    // Then: Will Log buffer size be set to max partion size
    uint64_t expected = AlignDownWithMetaPage(maxPartitionSize, &config);
    int numLogGroups = config.GetNumLogGroups();
    EXPECT_EQ(config.GetLogBufferSize(), expected);
    EXPECT_EQ(config.GetLogGroupSize(), expected / numLogGroups);

    delete configManager;
}

TEST(JournalConfiguration, SetLogBufferSize_testIfLogBufferSettedSuccessfully)
{
    // Given
    NiceMock<MockMetaFsFileControlApi> metaFsCtrl;
    uint64_t metaPageSize = 4032;
    uint64_t maxPartitionSize = 16 * SIZE_MB;
    uint64_t logBufferSizeInMB = maxPartitionSize / 2 / SIZE_MB;
    NiceMock<MockConfigManager> *configManager = CreateMockConfigManager(true, true, logBufferSizeInMB);
    JournalConfiguration config(configManager);

    ON_CALL(metaFsCtrl, EstimateAlignedFileIOSize).WillByDefault(Return(metaPageSize));
    ON_CALL(metaFsCtrl, GetAvailableSpace).WillByDefault(Return(maxPartitionSize));

    // When
    config.SetLogBufferSize(0, &metaFsCtrl);

    // Then: Will Log buffer size be set aligned to meta page size
    uint64_t expected = AlignDownWithMetaPage(logBufferSizeInMB * SIZE_MB, &config);
    int numLogGroups = config.GetNumLogGroups();
    EXPECT_EQ(config.GetLogBufferSize(), expected);
    EXPECT_EQ(config.GetLogGroupSize(), expected / numLogGroups);

    delete configManager;
}

TEST(JournalConfiguration, SetLogBufferSize_testIfLogBufferSetWhenLogBufferSizeIsBiggerThanPartitionSize)
{
    // Given: Log buffer size is bigger than max partition size
    NiceMock<MockMetaFsFileControlApi> metaFsCtrl;
    uint64_t metaPageSize = 4032;
    uint64_t maxPartitionSize = 16 * 1024 * 1024;

    uint64_t logBufferSizeInConfig = maxPartitionSize * 2;
    NiceMock<MockConfigManager> *configManager = CreateMockConfigManager(true, true, logBufferSizeInConfig);
    JournalConfiguration config(configManager);

    ON_CALL(metaFsCtrl, EstimateAlignedFileIOSize).WillByDefault(Return(metaPageSize));
    ON_CALL(metaFsCtrl, GetAvailableSpace).WillByDefault(Return(maxPartitionSize));

    // When
    config.SetLogBufferSize(0, &metaFsCtrl);

    // Then: Will Log buffer size be set to max partion size
    uint64_t expected = AlignDownWithMetaPage(maxPartitionSize, &config);
    int numLogGroups = config.GetNumLogGroups();
    EXPECT_EQ(config.GetLogBufferSize(), expected);
    EXPECT_EQ(config.GetLogGroupSize(), expected / numLogGroups);

    delete configManager;
}

TEST(JournalConfiguration, SetLogBufferSize_testIfNVRAMSpaceIsNotEnough)
{
    // Given: Max partition size is smaller than meta page size
    NiceMock<MockMetaFsFileControlApi> metaFsCtrl;
    NiceMock<MockConfigManager> *configManager = CreateMockConfigManager(true, true, 0);
    JournalConfiguration config(configManager);

    uint64_t metaPageSize = 4032;
    uint64_t maxPartitionSize = metaPageSize / 2;

    ON_CALL(metaFsCtrl, EstimateAlignedFileIOSize).WillByDefault(Return(metaPageSize));
    ON_CALL(metaFsCtrl, GetAvailableSpace).WillByDefault(Return(maxPartitionSize));

    // When, Then: Will journal configuration return the error code
    int errorReturnCode = static_cast<int>(POS_EVENT_ID::JOURNAL_CONFIGURATION) * -1;
    EXPECT_EQ(config.SetLogBufferSize(0, &metaFsCtrl), errorReturnCode);

    delete configManager;
}
} // namespace pos

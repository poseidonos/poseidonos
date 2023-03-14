/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "src/metafs/mim/io_statistics.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/unit-tests/metafs/config/metafs_config_manager_mock.h"
#include "test/unit-tests/metafs/lib/metafs_time_interval_mock.h"
#include "test/unit-tests/metafs/mim/mio_mock.h"
#include "test/unit-tests/metafs/mim/mpio_allocator_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

namespace pos
{
class IoStatisticsTestFixture : public ::testing::Test
{
public:
    IoStatisticsTestFixture(void)
    : tp(nullptr),
      conf(nullptr)
    {
    }

    virtual ~IoStatisticsTestFixture(void)
    {
    }

    virtual void SetUp(void)
    {
        tp = new NiceMock<MockTelemetryPublisher>;
        conf = new NiceMock<MockMetaFsConfigManager>(nullptr);
        EXPECT_CALL(*conf, GetMpioPoolCapacity).WillRepeatedly(Return(1024));
        EXPECT_CALL(*conf, GetSamplingSkipCount).WillRepeatedly(Return(2));
        EXPECT_CALL(*conf, GetTimeIntervalInMillisecondsForMetric).WillRepeatedly(Return(1000));

        allocator = new NiceMock<MockMpioAllocator>(conf);
        mio = new NiceMock<MockMio>(allocator);
        interval = new NiceMock<MockMetaFsTimeInterval>(conf->GetTimeIntervalInMillisecondsForMetric());

        stat = new IoStatistics(conf, interval, tp);
    }

    virtual void TearDown(void)
    {
        delete tp;
        delete conf;
        delete stat;
        delete mio;
    }

protected:
    IoStatistics* stat;

    NiceMock<MockTelemetryPublisher>* tp;
    NiceMock<MockMetaFsConfigManager>* conf;
    NiceMock<MockMpioAllocator>* allocator;
    NiceMock<MockMio>* mio;
    NiceMock<MockMetaFsTimeInterval>* interval;
};

TEST_F(IoStatisticsTestFixture, Constructor_testIfAllValuesAreInitialized)
{
    const int64_t INIT_VALUE = 0;

    {
        auto arr = stat->GetSampledTimeSpentProcessingAllStages();
        for (int i = 0; i < arr.size(); ++i)
            EXPECT_EQ(INIT_VALUE, arr[i]);
    }
    {
        auto arr = stat->GetSampledTimeSpentFromIssueToComplete();
        for (int i = 0; i < arr.size(); ++i)
            EXPECT_EQ(INIT_VALUE, arr[i]);
    }
    {
        auto arr = stat->GetTotalProcessedMioCount();
        for (int i = 0; i < arr.size(); ++i)
            EXPECT_EQ(INIT_VALUE, arr[i]);
    }
    {
        auto arr = stat->GetSampledProcessedMioCount();
        for (int i = 0; i < arr.size(); ++i)
            EXPECT_EQ(INIT_VALUE, arr[i]);
    }
    {
        auto arr = stat->GetIssueCountByStorage();
        for (int i = 0; i < arr.size(); ++i)
            for (int j = 0; j < arr[i].size(); ++j)
                EXPECT_EQ(INIT_VALUE, arr[i][j]);
    }
    {
        auto arr = stat->GetIssueCountByFileType();
        for (int i = 0; i < arr.size(); ++i)
            for (int j = 0; j < arr[i].size(); ++j)
                EXPECT_EQ(INIT_VALUE, arr[i][j]);
    }
}

TEST_F(IoStatisticsTestFixture, UpdateSubmission_testIfCountsOfReadIncresesCorrectly)
{
    const bool IS_READ = true;
    const MetaStorageType STORAGE_TYPE = MetaStorageType::JOURNAL_SSD;
    const MetaFileType FILE_TYPE = MetaFileType::Journal;

    EXPECT_CALL(*mio, IsRead).WillOnce(Return(IS_READ));
    EXPECT_CALL(*mio, GetTargetStorage).WillOnce(Return(STORAGE_TYPE));
    EXPECT_CALL(*mio, GetFileType).WillOnce(Return(FILE_TYPE));

    stat->UpdateSubmissionMetrics(mio);

    auto countOfStorage = stat->GetIssueCountByStorage();
    EXPECT_EQ(countOfStorage[(int)STORAGE_TYPE][(int)IS_READ], 1);

    auto countOfFile = stat->GetIssueCountByFileType();
    EXPECT_EQ(countOfFile[(int)FILE_TYPE][(int)IS_READ], 1);
}

TEST_F(IoStatisticsTestFixture, UpdateSubmission_testIfCountsOfWriteIncresesCorrectly)
{
    const bool IS_READ = false;
    const MetaStorageType STORAGE_TYPE = MetaStorageType::JOURNAL_SSD;
    const MetaFileType FILE_TYPE = MetaFileType::Journal;

    EXPECT_CALL(*mio, IsRead).WillOnce(Return(IS_READ));
    EXPECT_CALL(*mio, GetTargetStorage).WillOnce(Return(STORAGE_TYPE));
    EXPECT_CALL(*mio, GetFileType).WillOnce(Return(FILE_TYPE));

    stat->UpdateSubmissionMetrics(mio);

    auto countOfStorage = stat->GetIssueCountByStorage();
    EXPECT_EQ(countOfStorage[(int)STORAGE_TYPE][(int)IS_READ], 1);

    auto countOfFile = stat->GetIssueCountByFileType();
    EXPECT_EQ(countOfFile[(int)FILE_TYPE][(int)IS_READ], 1);
}

TEST_F(IoStatisticsTestFixture, UpdateCompletion_testIfSampledCountsOfReadIncresesCorrectly)
{
    const bool IS_READ = true;
    const MetaStorageType STORAGE_TYPE = MetaStorageType::JOURNAL_SSD;
    const MetaFileType FILE_TYPE = MetaFileType::Journal;
    const int ISSUE_COUNT = 50;

    EXPECT_CALL(*mio, IsRead).WillRepeatedly(Return(IS_READ));

    for (int i = 0; i < ISSUE_COUNT; ++i)
    {
        stat->UpdateCompletionMetricsConditionally(mio);
    }

    // total mio count
    EXPECT_EQ(stat->GetTotalProcessedMioCount()[(int)IS_READ], ISSUE_COUNT);
    // sampling count = issued_count / conf->GetSamplingSkipCount()
    EXPECT_EQ(stat->GetSampledProcessedMioCount()[(int)IS_READ], ISSUE_COUNT / 2);
}

TEST_F(IoStatisticsTestFixture, PublishPeriodicMetrics_testIfCannotPublishMetrics)
{
    EXPECT_CALL(*interval, CheckInterval).WillOnce(Return(false));
    EXPECT_CALL(*tp, PublishMetricList).Times(0);
    stat->PublishPeriodicMetrics(0, 0);
}

TEST_F(IoStatisticsTestFixture, PublishPeriodicMetrics_testIfPublishMetrics)
{
    EXPECT_CALL(*interval, CheckInterval).WillOnce(Return(true));
    EXPECT_CALL(*tp, PublishMetricList).WillOnce(Return(0));
    stat->PublishPeriodicMetrics(0, 0);
}
} // namespace pos

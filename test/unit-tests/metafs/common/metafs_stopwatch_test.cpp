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

#include "src/metafs/common/metafs_stopwatch.h"

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

namespace pos
{
enum class TestTimestampStage
{
    Stage_1,
    Stage_2,
    Stage_3,
    Stage_4,
    Stage_5,
    Stage_6,
    Count
};

TEST(MetaFsStopwatch, CheckStartAndStop)
{
    MetaFsStopwatch<TestTimestampStage> stopwatch;

    stopwatch.StoreTimestamp();
    usleep(2000);
    stopwatch.StoreTimestamp();

    // get raw data (nanoseconds on linux)
    auto v1 = stopwatch.GetDataInRaw();
    EXPECT_GE(v1.size(), 2);

    for (auto i : v1)
        std::cout << "raw: " << i.time_since_epoch().count() << std::endl;

    // get data in micro sec
    auto v2 = stopwatch.GetDataInMicro();
    EXPECT_GE(v2.size(), 2);

    for (auto i : v2)
        std::cout << "micro: " << i.count() << std::endl;

    // get data in milli sec
    auto v3 = stopwatch.GetDataInMilli();
    EXPECT_GE(v3.size(), 2);

    for (auto i : v3)
        std::cout << "milli: " << i.count() << std::endl;

    // get elapsed time from first Push()
    auto result_1st = stopwatch.GetElapsedInMilli();
    EXPECT_NE(result_1st.count(), 0);

    usleep(2000);
    stopwatch.StoreTimestamp();
    auto result_2nd = stopwatch.GetElapsedInMilli();
    EXPECT_NE(result_2nd.count(), result_1st.count());

    usleep(2000);
    stopwatch.StoreTimestamp();
    auto result_3rd = stopwatch.GetElapsedInMilli();
    EXPECT_NE(result_3rd.count(), result_2nd.count());
}

TEST(MetaFsStopwatch, GetElapsedInMilli_testIfTheReturnValueIsInitValue)
{
    MetaFsStopwatch<TestTimestampStage> stopwatch;
    EXPECT_EQ(0, stopwatch.GetElapsedInMilli().count());
}

TEST(MetaFsStopwatch, GetElapsedInMilli_testIfTheReturnValueIsInitValueEvenIfStoredTimeStampOnce)
{
    MetaFsStopwatch<TestTimestampStage> stopwatch;
    stopwatch.StoreTimestamp();
    EXPECT_EQ(0, stopwatch.GetElapsedInMilli().count());
}

TEST(MetaFsStopwatch, GetElapsedInMilli_testIfTheReturnValueIsNotInitValue)
{
    MetaFsStopwatch<TestTimestampStage> stopwatch;
    stopwatch.StoreTimestamp();
    usleep(1000);
    stopwatch.StoreTimestamp();
    EXPECT_NE(0, stopwatch.GetElapsedInMilli().count());
}

TEST(MetaFsStopwatch, CheckIfResetWillWorkCorrectly)
{
    const size_t SIZE = 5;
    MetaFsStopwatch<TestTimestampStage> stopwatch;

    for (size_t i = 0; i < SIZE; ++i)
        stopwatch.StoreTimestamp();

    auto result_1st = stopwatch.GetDataInRaw();
    EXPECT_EQ(result_1st.size(), SIZE);

    stopwatch.ResetTimestamp();

    auto result_2nd = stopwatch.GetDataInRaw();
    EXPECT_EQ(result_2nd.size(), 0);
}

TEST(MetaFsStopwatch, CheckIfTimestampWillBeStored)
{
    MetaFsStopwatch<TestTimestampStage> stopwatch;

    EXPECT_EQ(stopwatch.GetDataInRaw().size(), 0);

    for (size_t i = 0; i < (size_t)TestTimestampStage::Count; ++i)
    {
        stopwatch.StoreTimestamp((TestTimestampStage)i);
    }

    EXPECT_EQ(stopwatch.GetDataInRaw().size(), (size_t)TestTimestampStage::Count);
}

TEST(MetaFsStopwatch, CheckSparseTimestamp)
{
    MetaFsStopwatch<TestTimestampStage> stopwatch;

    EXPECT_EQ(stopwatch.GetDataInRaw().size(), 0);

    stopwatch.StoreTimestamp(TestTimestampStage::Stage_2);
    stopwatch.StoreTimestamp(TestTimestampStage::Stage_4);

    EXPECT_EQ(stopwatch.GetDataInRaw().size(), 4);

    auto result = stopwatch.GetDataInRaw();
    EXPECT_EQ(result[(size_t)TestTimestampStage::Stage_1].time_since_epoch().count(), 0);
    EXPECT_NE(result[(size_t)TestTimestampStage::Stage_2].time_since_epoch().count(), 0);
    EXPECT_EQ(result[(size_t)TestTimestampStage::Stage_3].time_since_epoch().count(), 0);
    EXPECT_NE(result[(size_t)TestTimestampStage::Stage_4].time_since_epoch().count(), 0);
}

TEST(MetaFsStopwatch, CheckInverseTimestamp)
{
    MetaFsStopwatch<TestTimestampStage> stopwatch;

    EXPECT_EQ(stopwatch.GetDataInRaw().size(), 0);

    stopwatch.StoreTimestamp(TestTimestampStage::Stage_4);
    EXPECT_EQ(stopwatch.GetDataInRaw().size(), 4);

    stopwatch.StoreTimestamp(TestTimestampStage::Stage_2);
    EXPECT_EQ(stopwatch.GetDataInRaw().size(), 4);

    auto result = stopwatch.GetDataInRaw();
    EXPECT_EQ(result[(size_t)TestTimestampStage::Stage_1].time_since_epoch().count(), 0);
    EXPECT_NE(result[(size_t)TestTimestampStage::Stage_2].time_since_epoch().count(), 0);
    EXPECT_EQ(result[(size_t)TestTimestampStage::Stage_3].time_since_epoch().count(), 0);
    EXPECT_NE(result[(size_t)TestTimestampStage::Stage_4].time_since_epoch().count(), 0);
}

TEST(MetaFsStopwatch, StoreTimestampWithStageName_CheckValidationOfElapsedTime)
{
    MetaFsStopwatch<TestTimestampStage> stopwatch;

    stopwatch.StoreTimestamp(TestTimestampStage::Stage_1);
    usleep(2000);
    stopwatch.StoreTimestamp(TestTimestampStage::Stage_2);

    auto result = stopwatch.GetElapsedInMilli(TestTimestampStage::Stage_1, TestTimestampStage::Stage_2);
    EXPECT_EQ(result.count(), 2);

    result = stopwatch.GetElapsedInMilli(TestTimestampStage::Stage_1, TestTimestampStage::Stage_3);
    EXPECT_EQ(result.count(), 0);

    result = stopwatch.GetElapsedInMilli(TestTimestampStage::Stage_2, TestTimestampStage::Stage_1);
    EXPECT_EQ(result.count(), 0);

    result = stopwatch.GetElapsedInMilli(TestTimestampStage::Stage_5, TestTimestampStage::Stage_1);
    EXPECT_EQ(result.count(), 0);
}

TEST(MetaFsStopwatch, StoreTimestamp_checkValidationOfElapsedTime)
{
    MetaFsStopwatch<TestTimestampStage> stopwatch;

    stopwatch.StoreTimestamp();
    usleep(2000);
    stopwatch.StoreTimestamp();
    usleep(2000);
    stopwatch.StoreTimestamp();
    usleep(2000);
    stopwatch.StoreTimestamp();
    usleep(2000);
    stopwatch.StoreTimestamp();
    usleep(2000);
    stopwatch.StoreTimestamp();

    auto result = stopwatch.GetDataInMilli();
    ASSERT_EQ(result.size(), (size_t)TestTimestampStage::Count);

    auto timestamp = result.at(0).count();
    for (auto i : result)
    {
        EXPECT_GE(i.count(), timestamp);
        timestamp = timestamp + 2;
    }
}

TEST(MetaFsStopwatch, StoreTimestamp_checkValidationOfElapsedTime2)
{
    MetaFsStopwatch<TestTimestampStage> stopwatch;

    stopwatch.StoreTimestamp();
    usleep(2000);
    stopwatch.StoreTimestamp();
    usleep(2000);
    stopwatch.StoreTimestamp();
    usleep(2000);
    stopwatch.StoreTimestamp();
    usleep(2000);
    stopwatch.StoreTimestamp();
    usleep(2000);
    stopwatch.StoreTimestamp();

    EXPECT_GE(stopwatch.GetElapsedInMilli(TestTimestampStage::Stage_1, TestTimestampStage::Stage_2).count(), 2);
    EXPECT_GE(stopwatch.GetElapsedInMilli(TestTimestampStage::Stage_2, TestTimestampStage::Stage_3).count(), 2);
    EXPECT_GE(stopwatch.GetElapsedInMilli(TestTimestampStage::Stage_3, TestTimestampStage::Stage_4).count(), 2);
    EXPECT_GE(stopwatch.GetElapsedInMilli(TestTimestampStage::Stage_4, TestTimestampStage::Stage_5).count(), 2);
}

TEST(MetaFsStopwatch, CheckElapsedTimeFromStageToStageWithStage)
{
    MetaFsStopwatch<TestTimestampStage> stopwatch;

    stopwatch.StoreTimestamp(TestTimestampStage::Stage_1);
    usleep(2000);
    stopwatch.StoreTimestamp(TestTimestampStage::Stage_2);
    usleep(2000);
    stopwatch.StoreTimestamp(TestTimestampStage::Stage_3);
    usleep(2000);
    stopwatch.StoreTimestamp(TestTimestampStage::Stage_4);
    usleep(2000);
    stopwatch.StoreTimestamp(TestTimestampStage::Stage_5);
    usleep(2000);
    stopwatch.StoreTimestamp(TestTimestampStage::Stage_6);

    EXPECT_GE(stopwatch.GetElapsedInMilli(TestTimestampStage::Stage_1, TestTimestampStage::Stage_2).count(), 2);
    EXPECT_GE(stopwatch.GetElapsedInMilli(TestTimestampStage::Stage_2, TestTimestampStage::Stage_3).count(), 2);
    EXPECT_GE(stopwatch.GetElapsedInMilli(TestTimestampStage::Stage_3, TestTimestampStage::Stage_4).count(), 2);
    EXPECT_GE(stopwatch.GetElapsedInMilli(TestTimestampStage::Stage_4, TestTimestampStage::Stage_5).count(), 2);
}
} // namespace pos

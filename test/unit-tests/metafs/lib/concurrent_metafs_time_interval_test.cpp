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

#include "src/metafs/lib/concurrent_metafs_time_interval.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <future>
#include <thread>

namespace pos
{
TEST(ConcurrentMetaFsTimeInterval, IsExceedTimeInterval)
{
    size_t TIME_INTERVAL_IN_MILLISECONDS = 200; // 0.2s
    ConcurrentMetaFsTimeInterval interval(TIME_INTERVAL_IN_MILLISECONDS);
    EXPECT_FALSE(interval.CheckInterval());
    usleep(100000);
    EXPECT_FALSE(interval.CheckInterval());
    usleep(200000);
    EXPECT_TRUE(interval.CheckInterval());
    EXPECT_FALSE(interval.CheckInterval());
}

TEST(ConcurrentMetaFsTimeInterval, IsConcurrentlyWorking)
{
    // Given
    size_t TIME_INTERVAL_IN_MILLISECONDS = 200; // 0.2s
    std::chrono::steady_clock::time_point lastTime1, lastTime2;
    bool ret1, ret2;
    ConcurrentMetaFsTimeInterval interval(TIME_INTERVAL_IN_MILLISECONDS);

    std::thread thread1([&] { ret1 = interval.CheckInterval(); });
    std::thread thread2([&] { ret2 = interval.CheckInterval(); });

    // No thread can get true before interval (0.2s)
    thread1.join();
    EXPECT_FALSE(ret1);
    usleep(100000);
    thread2.join();
    EXPECT_FALSE(ret2);

    // When : after 0.2s interval
    // Premise : start timing difference between thread3 and thread4 must be within 0.2s
    usleep(200000);
    std::thread thread3([&] {ret1 = interval.CheckInterval(); lastTime1 = interval.GetLastTime(); });
    std::thread thread4([&] {ret2 = interval.CheckInterval(); lastTime2 = interval.GetLastTime(); });
    thread3.join();
    thread4.join();

    // Then : Only one thread can change value of lastTime_ variable because this variable is in critical section.
    EXPECT_NE(ret1, ret2);

    // Then : after changing value of lastTime_ , no other thread can change lastTime value. So, lastTime values must be same based on premise.
    EXPECT_EQ(lastTime1, lastTime2);
}
} // namespace pos

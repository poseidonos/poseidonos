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

#pragma once

#include <src/metafs/lib/metafs_time_interval.h>
#include <tbb/spin_rw_mutex.h>

#include <chrono>
#include <iostream>
#include <thread>

namespace pos
{
class ConcurrentMetaFsTimeInterval : public MetaFsTimeInterval
{
public:
    ConcurrentMetaFsTimeInterval(void) = delete;
    explicit ConcurrentMetaFsTimeInterval(const int64_t intervalInMilliseconds)
    : MetaFsTimeInterval(intervalInMilliseconds)
    {
        bool isWrite = true;
        tbb::spin_rw_mutex_v3::scoped_lock rwLock(mutex, isWrite);
        lastTime_ = std::chrono::steady_clock::now();
    }
    virtual ~ConcurrentMetaFsTimeInterval(void)
    {
    }
    virtual bool CheckInterval(void)
    {
        bool isWrite = false;
        tbb::spin_rw_mutex_v3::scoped_lock rwLock(mutex, isWrite);

        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime_).count();

        if (elapsedTime >= intervalInMilliseconds_)
        {
            rwLock.upgrade_to_writer();
            lastTime_ = currentTime;
            return true;
        }
        return false;
    }
    // Use this method only for UT
    virtual std::chrono::steady_clock::time_point GetLastTime(void)
    {
        return lastTime_;
    }

private:
    tbb::spin_rw_mutex_v3 mutex;
};
} // namespace pos

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

#include <chrono>
#include <iostream>
#include <vector>

namespace pos
{
template<class StageEnum>
class MetaFsStopwatch
{
public:
    MetaFsStopwatch(void);
    virtual ~MetaFsStopwatch(void);
    virtual void StoreTimestamp(void);
    virtual void StoreTimestamp(StageEnum stage);
    virtual void ResetTimestamp(void);
    virtual std::vector<std::chrono::system_clock::time_point> GetDataInRaw(void) const;
    virtual std::vector<std::chrono::milliseconds> GetDataInMilli(void) const;
    virtual std::vector<std::chrono::nanoseconds> GetDataInNano(void) const;
    virtual std::vector<std::chrono::seconds> GetDataInSec(void) const;
    virtual std::chrono::milliseconds GetElapsedInMilli(void) const;
    virtual std::chrono::milliseconds GetElapsedInMilli(StageEnum from, StageEnum to) const;

private:
    std::vector<std::chrono::system_clock::time_point> stamp_;
};

template<class StageEnum>
MetaFsStopwatch<StageEnum>::MetaFsStopwatch(void)
{
    stamp_.reserve((size_t)StageEnum::Count);
}

template<class StageEnum>
MetaFsStopwatch<StageEnum>::~MetaFsStopwatch(void)
{
}

template<class StageEnum>
void
MetaFsStopwatch<StageEnum>::StoreTimestamp(void)
{
    if ((size_t)StageEnum::Count <= stamp_.size())
        return;

    stamp_.emplace_back(std::chrono::system_clock::now());
}

template<class StageEnum>
void
MetaFsStopwatch<StageEnum>::StoreTimestamp(StageEnum stage)
{
    const size_t index = (size_t)stage;
    while (stamp_.size() <= index)
    {
        stamp_.emplace_back(std::chrono::system_clock::time_point{});
    }
    stamp_[index] = std::chrono::system_clock::now();
}

template<class StageEnum>
void
MetaFsStopwatch<StageEnum>::ResetTimestamp(void)
{
    stamp_.clear();
}

template<class StageEnum>
std::vector<std::chrono::system_clock::time_point>
MetaFsStopwatch<StageEnum>::GetDataInRaw(void) const
{
    std::vector<std::chrono::system_clock::time_point> result;
    for (auto& i : stamp_)
        result.emplace_back(i);
    return result;
}

template<class StageEnum>
std::vector<std::chrono::milliseconds>
MetaFsStopwatch<StageEnum>::GetDataInMilli(void) const
{
    std::vector<std::chrono::milliseconds> result;
    for (auto& i : stamp_)
        result.emplace_back(std::chrono::duration_cast<std::chrono::milliseconds>(i.time_since_epoch()));
    return result;
}

template<class StageEnum>
std::vector<std::chrono::nanoseconds>
MetaFsStopwatch<StageEnum>::GetDataInNano(void) const
{
    std::vector<std::chrono::nanoseconds> result;
    for (auto& i : stamp_)
        result.emplace_back(std::chrono::duration_cast<std::chrono::nanoseconds>(i.time_since_epoch()));
    return result;
}

template<class StageEnum>
std::vector<std::chrono::seconds>
MetaFsStopwatch<StageEnum>::GetDataInSec(void) const
{
    std::vector<std::chrono::seconds> result;
    for (auto& i : stamp_)
        result.emplace_back(std::chrono::duration_cast<std::chrono::seconds>(i.time_since_epoch()));
    return result;
}

template<class StageEnum>
std::chrono::milliseconds
MetaFsStopwatch<StageEnum>::GetElapsedInMilli(void) const
{
    if (stamp_.size() <= 1)
        return {};
    return std::chrono::duration_cast<std::chrono::milliseconds>(stamp_.back() - stamp_.front());
}

template<class StageEnum>
std::chrono::milliseconds
MetaFsStopwatch<StageEnum>::GetElapsedInMilli(StageEnum from, StageEnum to) const
{
    if ((to <= from) ||
        (stamp_[(size_t)to].time_since_epoch().count() == 0) ||
        (stamp_[(size_t)from].time_since_epoch().count() == 0))
        return {};
    return std::chrono::duration_cast<std::chrono::milliseconds>(stamp_[(size_t)to] - stamp_[(size_t)from]);
}
} // namespace pos

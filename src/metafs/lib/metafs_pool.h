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

#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace pos
{
/* thread unsafe, 'Value' should be a pointer type of certain object */
template<class T>
class MetaFsPool
{
public:
    MetaFsPool(void) = delete;
    explicit MetaFsPool(const size_t poolSize)
    : CAPACITY(poolSize),
      usedCount_(0)
    {
        all_.reserve(CAPACITY);
    }
    virtual ~MetaFsPool(void)
    {
        for (auto i : all_)
            delete i;
        all_.clear();
        free_.clear();
    }
    virtual bool AddToPool(T item)
    {
        if (GetCapacity() <= (GetFreeCount() + GetUsedCount()))
            return false;
        free_.emplace_back(item);
        all_.emplace_back(item);
        return true;
    }
    virtual T TryAlloc(void)
    {
        if (free_.empty())
            return nullptr;
        auto item = free_.front();
        free_.pop_front();
        used_.emplace(item);
        usedCount_++;
        return item;
    }
    virtual void Release(T item)
    {
        free_.emplace_back(item);
        used_.erase(item);
        usedCount_--;
    }
    virtual size_t GetCapacity(void) const
    {
        return CAPACITY;
    }
    virtual size_t GetFreeCount(void) const
    {
        return free_.size();
    }
    virtual size_t GetUsedCount(void) const
    {
        return all_.size() - GetFreeCount();
    }

private:
    const size_t CAPACITY;
    std::list<T> free_;
    std::vector<T> all_;
    std::unordered_set<T> used_;
    std::size_t usedCount_;
};
} // namespace pos

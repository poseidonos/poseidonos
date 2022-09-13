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
#include <list>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "src/include/pos_event_id.h"
#include "src/metafs/log/metafs_log.h"

namespace pos
{
struct MakeHash
{
    template<typename Key_1, typename Key_2>
    size_t operator()(const std::pair<Key_1, Key_2>& pair) const
    {
        return std::hash<Key_1>{}(pair.first) ^ std::hash<Key_2>{}(pair.second);
    };
};

/* thread unsafe, 'Value' should be a pointer type of certain object */
template<typename Key_1, typename Key_2, typename Value>
class FifoCache
{
public:
    FifoCache(void) = delete;
    explicit FifoCache(const size_t cacheSize)
    : CAPACITY(cacheSize)
    {
    }
    virtual ~FifoCache(void)
    {
        map_.clear();
        cache_.clear();
    }
    virtual Value Find(const std::pair<Key_1, Key_2>& key)
    {
        auto iter = map_.find(key);
        if (iter == map_.end())
            return nullptr;
        return iter->second;
    }
    /* If the key already exists in the cache, this will return nullptr */
    virtual Value Push(const std::pair<Key_1, Key_2>& key, const Value& value)
    {
        if (map_.find(key) != map_.end())
        {
            POS_TRACE_ERROR(EID(MFS_INVALID_PARAMETER),
                "the key pair is already existed.");
            return nullptr;
        }
        else
        {
            Value victim = IsFull() ? _PopTheOldest() : nullptr;
            _Push(key, value);
            return victim;
        }
    }
    virtual Value PopTheOldest(void)
    {
        if (!cache_.size())
            return nullptr;

        return _PopTheOldest();
    }
    virtual Value Remove(const std::pair<Key_1, Key_2>& key)
    {
        return _Remove(key);
    }
    virtual size_t GetCapacity(void) const
    {
        return CAPACITY;
    }
    virtual bool IsEmpty(void) const
    {
        return (0 == cache_.size());
    }
    virtual bool IsFull(void) const
    {
        return (CAPACITY == cache_.size());
    }
    virtual size_t GetSize(void) const
    {
        return cache_.size();
    }

private:
    void _Push(const std::pair<Key_1, Key_2>& key, const Value v)
    {
        cache_.push_back(std::make_tuple(key.first, key.second, v));
        map_.insert({key, v});
    }
    Value _PopTheOldest(void)
    {
        auto item = cache_.front();
        cache_.pop_front();
        map_.erase(std::make_pair(std::get<0>(item), std::get<1>(item)));
        return std::get<2>(item);
    }
    Value _Remove(const std::pair<Key_1, Key_2>& key)
    {
        Value v = nullptr;
        for (auto iter = cache_.begin(); iter != cache_.end(); ++iter)
        {
            if (std::get<0>(*iter) == key.first && std::get<1>(*iter) == key.second)
            {
                v = std::get<2>(*iter);
                cache_.erase(iter);
                break;
            }
        }
        map_.erase(key);
        return v;
    }

    std::unordered_map<std::pair<Key_1, Key_2>, Value, MakeHash> map_;
    /* "cache_" will perform a delete with O(N) time complexity, which should be okay given that the list size will be small enough.
    The size should not exceed 32 (or so). */
    std::list<std::tuple<Key_1, Key_2, Value>> cache_;
    const size_t CAPACITY;
};
} // namespace pos

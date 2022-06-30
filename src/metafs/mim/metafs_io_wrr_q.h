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

#include <array>
#include <algorithm>
#include <vector>

#include "metafs_io_q.h"

namespace pos
{
/* 'ItemT' should be a pointer type of certain object */
template<typename ItemT, class TypeE>
class MetaFsIoWrrQ
{
public:
    MetaFsIoWrrQ(void)
    : MetaFsIoWrrQ(_CreateDefaultWeight())
    {
    }
    MetaFsIoWrrQ(const std::vector<int> weight)
    : weight_(weight),
      index_(0),
      remainedWeight_(weight[0])
    {
    }
    /* All items in queue will be deleted */
    virtual ~MetaFsIoWrrQ(void)
    {
        ItemT t;
        while (nullptr != (t = Dequeue()))
        {
            delete t;
        }
    }
    /* thread safe */
    virtual void Enqueue(const ItemT entry, const TypeE type)
    {
        queue_[(size_t)type].Enqueue(entry);
    }
    /* thread unsafe */
    virtual ItemT Dequeue(void)
    {
        for (size_t cnt = 0; cnt < TYPE_COUNT; ++cnt)
        {
            ItemT t = queue_[index_].Dequeue();
            if (nullptr != t)
            {
                _DecreaseCurrentWeight();
                return t;
            }
            _MoveToNextIndex();
        }
        return nullptr;
    }
    // for test
    virtual void SetStartIndex(size_t index)
    {
        index_ = index;
        remainedWeight_ = weight_[index];
    }
    virtual void SetWeight(const std::vector<int> weight)
    {
        weight_ = weight;
    }

private:
    void _MoveToNextIndex(void)
    {
        index_++;
        if (index_ >= TYPE_COUNT)
            index_ = 0;
        remainedWeight_ = weight_[index_];
    }
    void _DecreaseCurrentWeight(void)
    {
        remainedWeight_--;
        if (remainedWeight_ <= 0)
            _MoveToNextIndex();
    }
    const std::vector<int> _CreateDefaultWeight(void) const
    {
        std::vector<int> weight(TYPE_COUNT);
        std::fill(weight.begin(), weight.begin() + TYPE_COUNT, 1);
        return weight;
    }

    static const size_t TYPE_COUNT = (size_t)TypeE::MAX;
    std::array<MetaFsIoQ<ItemT>, TYPE_COUNT> queue_;
    std::vector<int> weight_;
    size_t index_;
    int remainedWeight_;
};
} // namespace pos

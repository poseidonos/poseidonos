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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "metafs_log.h"
#include "rte_ring.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
enum class LockLessQType
{
    MPMC = 0, // Multi-producer, Multi-consumer
    MPSC,     // Multi-producer, Single-consumer
    SPSC,     // Single-producer, Single-consumer
};

template<class Entity> // Entity  should be a pointer type of certain object
class MetaFsLockLessQ
{
public:
    MetaFsLockLessQ(void)
    : ring(nullptr),
      qSize(0)
    {
    }

    void
    Init(const char* qName, LockLessQType type, uint32_t numEntries, int coreId)
    {
        if (ring)
        {
            rte_ring_free(ring);
        }
        qSize = numEntries;
        uint32_t flags = 0;
        switch (type)
        {
            case LockLessQType::SPSC:
                flags = RING_F_SP_ENQ | RING_F_SC_DEQ;
                break;
            case LockLessQType::MPSC:
                flags = RING_F_SC_DEQ;
                break;
            case LockLessQType::MPMC:
                flags = 0;
                break;
            default:
                MFS_TRACE_CRITICAL((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
                    "Given QType is wrong...(type={})", (int)type);
                assert(false);
        }
        flags |= RING_F_EXACT_SZ;
        ring = rte_ring_create(qName, numEntries, coreId, flags);
        assert(ring != NULL);

        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "RTE Ring has been created. name={}, numEntries={}, cpu_id={}, flags={}",
            qName, numEntries, coreId, flags);
    }
    ~MetaFsLockLessQ(void)
    {
        rte_ring_free(ring);
    }

    bool
    Push(Entity item)
    {
        int ret;
        assert(ring != NULL);
        ret = rte_ring_enqueue(ring, item);

        if (ret)
        {
            MFS_TRACE_CRITICAL((int)POS_EVENT_ID::MFS_QUEUE_PUSH_FAILED,
                "Item push failed. rc={}", ret);
            assert(false);
            return false;
        }
        return true;
    }
    Entity
    Pop(void)
    {
        Entity item = nullptr;
        int ret;
        assert(ring != NULL);
        ret = rte_ring_dequeue(ring, (void**)&item);

        if (ret)
        {
            MFS_TRACE_CRITICAL((int)POS_EVENT_ID::MFS_QUEUE_POP_FAILED,
                "Item pop failed. rc={}", ret);
            assert(false);
            return item;
        }
        return item;
    }
    bool
    IsEmpty(void)
    {
        assert(ring != NULL);
        return rte_ring_empty(ring);
    }

    int
    GetItemCount(void)
    {
        assert(ring != NULL);
        return rte_ring_count(ring);
    }

private:
    rte_ring* ring;
    uint32_t qSize;
};
} // namespace pos

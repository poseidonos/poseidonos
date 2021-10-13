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

#include <queue>
#include <utility>

#include "mfs_lockless_q.h"
#include "metafs_spinlock.h"
#include "os_header.h"

#if defined UNVME_BUILD
#define RTE_LOCKLESS_Q 0 // need SPDK/DPDK rte libraries.
#else
#define RTE_LOCKLESS_Q 0 // for UTs.
#endif

namespace pos
{
/* MetaFsIoQ is a general purpose lock-less Q adopting DPDK RTE Ring library */
template<typename T> // 'T' should be a pointer type of certain object
class MetaFsIoQ
{
public:
    MetaFsIoQ(void);
    explicit MetaFsIoQ(uint32_t weight);
    virtual ~MetaFsIoQ(void);

    virtual void Init(const char* qName, uint32_t numEntries);
    virtual bool IsAllQEmpty(void);
    virtual bool IsEmpty(void);
    virtual uint32_t GetItemCnt(void);
    virtual void SetWeightFactor(uint32_t weight);
    virtual uint32_t GetWeightFactor(void);
    virtual bool Enqueue(T obj);
    virtual T Dequeue(void);
    virtual void CleanQEntry(void);

private:
#if (1 == RTE_LOCKLESS_Q)
    MetaFsLockLessQ<T> q;
#else
    std::queue<T> q;
#endif

    uint32_t weightFactor;
    MetaFsSpinLock qlock;
    uint32_t elements;
};

template<typename T>
MetaFsIoQ<T>::MetaFsIoQ(void)
: weightFactor(0),
  elements(0)
{
}

template<typename T>
MetaFsIoQ<T>::MetaFsIoQ(uint32_t weight)
: weightFactor(weight)
{
}

template<typename T>
MetaFsIoQ<T>::~MetaFsIoQ(void)
{
}

template<typename T>
void
MetaFsIoQ<T>::Init(const char* qName, uint32_t numEntries)
{
#if (1 == RTE_LOCKLESS_Q)
    q.Init(qName, LockLessQType::MPMC, numEntries, rte_socket_id());
#endif
}

template<typename T>
void
MetaFsIoQ<T>::SetWeightFactor(uint32_t weight)
{
    this->weightFactor = weight;
}

template<typename T>
uint32_t
MetaFsIoQ<T>::GetWeightFactor(void)
{
    return weightFactor;
}

template<typename T>
bool
MetaFsIoQ<T>::IsAllQEmpty(void)
{
    return IsEmpty();
}

template<typename T>
bool
MetaFsIoQ<T>::IsEmpty(void)
{
#if (1 == RTE_LOCKLESS_Q)
    return q.IsEmpty();
#else
    SPIN_LOCK_GUARD_IN_SCOPE(qlock);
    return q.empty();
#endif
}

template<typename T>
uint32_t
MetaFsIoQ<T>::GetItemCnt(void)
{
#if (1 == RTE_LOCKLESS_Q)
    return q.GetItemCount();
#else
    SPIN_LOCK_GUARD_IN_SCOPE(qlock);
    return q.size();
#endif
}

template<typename T>
bool
MetaFsIoQ<T>::Enqueue(T obj)
{
#if (1 == RTE_LOCKLESS_Q)
    q.Push(obj);
#else
    SPIN_LOCK_GUARD_IN_SCOPE(qlock);
    q.push(obj);
#endif
    elements++;

    return true;
}

template<typename T>
T
MetaFsIoQ<T>::Dequeue(void)
{
#if (1 == RTE_LOCKLESS_Q)
    if (q.IsEmpty())
    {
        return nullptr;
    }
    T obj = q.Pop();
    elements--;
#else

    SPIN_LOCK_GUARD_IN_SCOPE(qlock);
    if (q.empty())
    {
        return nullptr;
    }
    T obj = q.front();
    if (obj)
    {
        q.pop();
        elements--;
    }
#endif
    return obj;
}

template<typename T>
void
MetaFsIoQ<T>::CleanQEntry(void)
{
    SPIN_LOCK_GUARD_IN_SCOPE(qlock);

    std::queue<T> empty;
    std::swap(q, empty);

    elements = 0;
}
} // namespace pos

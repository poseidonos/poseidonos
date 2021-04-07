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

#include "meta_io_req_q.h"

#include "mfs_mutex.h"

MetaIoQ::MetaIoQ(void)
{
}

bool
MetaIoQ::IsEmpty(void)
{
    return q.empty();
}

bool
MetaIoQ::Enqueue(MetaFsIoReqMsg* msg)
{
    SPIN_LOCK_GUARD_IN_SCOPE(qlock);
    q.push_back(msg);
    return true;
}

MetaFsIoReqMsg*
MetaIoQ::Dequeue(void)
{
    SPIN_LOCK_GUARD_IN_SCOPE(qlock);
    if (unlikely(true == IsEmpty()))
    {
        return nullptr;
    }

    MetaFsIoReqMsg* msg = q.front();
    if (msg != nullptr)
    {
        q.pop_front();
    }

    return msg;
}

MetaIoMultiQ::MetaIoMultiQ(void)
{
    for (uint32_t index = 0; index < MetaFsConfig::DEFAULT_MAX_CORE_COUNT; index++)
    {
        msgQ[index] = nullptr;
    }
}

MetaIoMultiQ::~MetaIoMultiQ(void)
{
    Clear();
}

void
MetaIoMultiQ::Init(void)
{
}

void
MetaIoMultiQ::Clear(void)
{
    for (uint32_t index = 0; index < MetaFsConfig::DEFAULT_MAX_CORE_COUNT; index++)
    {
        if (nullptr != msgQ[index])
        {
            delete msgQ[index];
            msgQ[index] = nullptr;
        }
    }
}

bool
MetaIoMultiQ::EnqueueReqMsg(uint32_t coreId, MetaFsIoReqMsg& reqMsg)
{
    MetaIoQ* reqMsgQ = msgQ[coreId];

    if (unlikely(nullptr == reqMsgQ))
    {
        MetaIoQ* newReqMsgQ = new MetaIoQ();
        msgQ[coreId] = newReqMsgQ;
        reqMsgQ = newReqMsgQ;
    }

    bool mioQueued = reqMsgQ->Enqueue(&reqMsg);

    return mioQueued;
}

bool
MetaIoMultiQ::IsEmpty(int coreId)
{
    MetaIoQ* reqMsgQ = msgQ[coreId];

    if (nullptr == reqMsgQ)
        return true;

    return msgQ[coreId]->IsEmpty();
}

MetaFsIoReqMsg*
MetaIoMultiQ::DequeueReqMsg(uint32_t coreId)
{
    MetaIoQ* reqMsgQ = msgQ[coreId];
    if (nullptr == reqMsgQ)
    {
        return nullptr;
    }

    MetaFsIoReqMsg* reqMsg = reqMsgQ->Dequeue();

    return reqMsg;
}

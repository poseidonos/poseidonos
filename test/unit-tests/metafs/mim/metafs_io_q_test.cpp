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

#include "src/metafs/mim/metafs_io_q.h"

#include <gtest/gtest.h>

#include <unordered_set>

#include "src/metafs/mim/metafs_io_request.h"
#include "test/unit-tests/metafs/config/metafs_config_manager_mock.h"
#include "test/unit-tests/metafs/mim/mio_mock.h"
#include "test/unit-tests/metafs/mim/mpio_allocator_mock.h"
#include "test/unit-tests/metafs/mim/mpio_mock.h"

using ::testing::Return;

namespace pos
{
/*** MetaFsIoRequest* ***/
TEST(MetaFsIoQ_Msg, ConstructorAndDestructor)
{
    MetaFsIoQ<MetaFsIoRequest*>* q = new MetaFsIoQ<MetaFsIoRequest*>();
    delete q;
}

TEST(MetaFsIoQ_Msg, CheckEnqueueAndDequeue)
{
    const int SIZE = 200;
    MetaFsIoQ<MetaFsIoRequest*> q;
    std::unordered_set<MetaFsIoRequest*> requests;

    for (int i = 0; i < SIZE; i++)
    {
        MetaFsIoRequest* msg = new MetaFsIoRequest();
        q.Enqueue(msg);
        requests.insert(msg);
    }

    EXPECT_FALSE(q.IsEmpty());

    MetaFsIoRequest* msg = nullptr;
    while (nullptr != (msg = q.Dequeue()))
    {
        EXPECT_NE(requests.find(msg), requests.end());
        requests.erase(msg);
        delete msg;
    }

    EXPECT_TRUE(q.IsEmpty());
}

/*** Mio* ***/
TEST(MetaFsIoQ_Mio, ConstructorAndDestructor)
{
    MetaFsIoQ<Mio*>* q = new MetaFsIoQ<Mio*>();
    delete q;
}

TEST(MetaFsIoQ_Mio, CheckEnqueueAndDequeue)
{
    const int SIZE = 200;
    MetaFsIoQ<Mio*> q;
    std::unordered_set<Mio*> requests;
    MockMetaFsConfigManager* conf = new MockMetaFsConfigManager(nullptr);
    EXPECT_CALL(*conf, GetMpioPoolCapacity).WillRepeatedly(Return(SIZE));
    EXPECT_CALL(*conf, GetWriteMpioCacheCapacity).WillRepeatedly(Return(SIZE));

    MockMpioAllocator* allocator = new MockMpioAllocator(conf);

    for (int i = 0; i < SIZE; i++)
    {
        MockMio* msg = new MockMio(allocator);
        q.Enqueue(msg);
        requests.insert(msg);
    }

    EXPECT_FALSE(q.IsEmpty());

    MockMio* msg = nullptr;
    while (nullptr != (msg = dynamic_cast<MockMio*>(q.Dequeue())))
    {
        EXPECT_NE(requests.find(msg), requests.end());
        requests.erase(msg);
        delete msg;
    }

    EXPECT_TRUE(q.IsEmpty());

    delete conf;
}

/*** Mpio* ***/
TEST(MetaFsIoQ_Mpio, ConstructorAndDestructor)
{
    MetaFsIoQ<Mpio*>* q = new MetaFsIoQ<Mpio*>();
    delete q;
}

TEST(MetaFsIoQ_Mpio, CheckEnqueueAndDequeue)
{
    const int SIZE = 200;
    MetaFsIoQ<Mpio*> q;
    std::unordered_set<Mpio*> requests;

    for (int i = 0; i < SIZE; i++)
    {
        MockMpio* msg = new MockMpio(nullptr, false);
        q.Enqueue(msg);
        requests.insert(msg);
    }

    EXPECT_FALSE(q.IsEmpty());

    MockMpio* msg = nullptr;
    while (nullptr != (msg = dynamic_cast<MockMpio*>(q.Dequeue())))
    {
        EXPECT_NE(requests.find(msg), requests.end());
        requests.erase(msg);
        delete msg;
    }

    EXPECT_TRUE(q.IsEmpty());
}
} // namespace pos

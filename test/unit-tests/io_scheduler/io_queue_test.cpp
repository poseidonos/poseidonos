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

#include "src/io_scheduler/io_queue.h"

#include <gtest/gtest.h>

namespace pos
{
} // namespace pos

namespace pos
{
TEST(IOQueue, IOQueue_Stack)
{
    // Given: Do nothing

    // When: Create IOQueue
    IOQueue ioQueue;

    // Then: Do nothing
}

TEST(IOQueue, IOQueue_Heap)
{
    // Given: Do nothing

    // When: Create IOQueue
    IOQueue* ioQueue = new IOQueue;
    delete ioQueue;

    // Then: Do nothing
}

TEST(IOQueue, EnqueueUbio_InputNullptr)
{
    // Given: IOQueue
    IOQueue ioQueue;

    // When: Call EnqueueUbio with nullptr
    ioQueue.EnqueueUbio(nullptr);

    // Then: Expect to get zero(queue size)
    EXPECT_EQ(0, ioQueue.GetQueueSize());
}

TEST(IOQueue, EnqueueUbio_InputUbio)
{
    // Given: IOQueue, UbioSmartPtr
    IOQueue ioQueue;
    auto ubio = std::make_shared<Ubio>(nullptr, 0, 0);

    // When: Call EnqueueUbio with ubio
    ioQueue.EnqueueUbio(ubio);

    // Then: Expect to get one(queue size)
    EXPECT_EQ(1, ioQueue.GetQueueSize());
}

TEST(IOQueue, DequeueUbio_EmptyQueue)
{
    // Given: IOQueue
    IOQueue ioQueue;
    UbioSmartPtr actual, expected = nullptr;

    // When: Call DequeueUbio with empty queue
    actual = ioQueue.DequeueUbio();

    // Then: Expect to return nullptr
    EXPECT_EQ(actual, expected);
}

TEST(IOQueue, DequeueUbio_NotEmptyQueue)
{
    // Given: IOQueue, UbioSmartPtr
    IOQueue ioQueue;
    auto ubio1 = std::make_shared<Ubio>(nullptr, 0, 0);
    auto ubio2 = std::make_shared<Ubio>(nullptr, 0, 0);
    ioQueue.EnqueueUbio(ubio1);
    ioQueue.EnqueueUbio(ubio2);
    UbioSmartPtr result;

    // When 1: Call DequeueUbio
    result = ioQueue.DequeueUbio();

    // Then 1: Expect to return ubio1
    EXPECT_EQ(result.get(), ubio1.get());

    // When 2: Call DequeueUbio
    result = ioQueue.DequeueUbio();

    // Then 2: Expect to return ubio2
    EXPECT_EQ(result.get(), ubio2.get());
}

TEST(IOQueue, GetQueueSize_EnqTwice_DeqOnce)
{
    // Given: IOQueue, UbioSmartPtr
    IOQueue ioQueue;
    auto ubio1 = std::make_shared<Ubio>(nullptr, 0, 0);
    auto ubio2 = std::make_shared<Ubio>(nullptr, 0, 0);
    ioQueue.EnqueueUbio(ubio1);
    ioQueue.EnqueueUbio(ubio2);
    ioQueue.DequeueUbio();
    int actual, expected = 1;

    // When: Call GetQueueSize
    actual = ioQueue.GetQueueSize();

    // Then: Expect to return 1
    EXPECT_EQ(actual, expected);
}

} // namespace pos

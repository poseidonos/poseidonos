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

#include "src/io_scheduler/io_worker_device_operation_queue.h"

#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <unistd.h>

namespace pos
{
TEST(IoWorkerDeviceOperationQueue, IoWorkerDeviceOperationQueue_Stack)
{
    // Given: Do nothing

    // When: Create IoWorkerDeviceOperationQueue
    IoWorkerDeviceOperationQueue ioWorkerDeviceOperationQueue;

    // Then: Do nothing
}

TEST(IoWorkerDeviceOperationQueue, IoWorkerDeviceOperationQueue_Heap)
{
    // Given: Do nothing

    // When: Create IoWorkerDeviceOperationQueue
    IoWorkerDeviceOperationQueue* ioWorkerDeviceOperationQueue = new IoWorkerDeviceOperationQueue;

    // Then: Do nothing
}

TEST(IoWorkerDeviceOperationQueue, SubmitAndWait_SimpleCall)
{
    // Given: IoWorkerDeviceOperationQueue, t1
    IoWorkerDeviceOperationQueue ioWorkerDeviceOperationQueue;
    std::thread t1([&](void) -> void
    {
        usleep(10000); // 10ms
        auto iwdOperation = ioWorkerDeviceOperationQueue.Pop();
        while (nullptr == iwdOperation)
        {
            usleep(1);
            iwdOperation = ioWorkerDeviceOperationQueue.Pop();
        }
        iwdOperation->SetDone();
    });

    // When: Call SubmitAndWait
    ioWorkerDeviceOperationQueue.SubmitAndWait(IoWorkerDeviceOperationType::INSERT, nullptr);
    t1.join();

    // Then: Do nothing
}

TEST(IoWorkerDeviceOperationQueue, Pop_ThreeThreadsParallelRun)
{
    // Given: IoWorkerDeviceOperationQueue, t1, t2, t3, atomic<int>
    IoWorkerDeviceOperationQueue ioWorkerDeviceOperationQueue;
    std::atomic<int> pop_count {0};
    auto pop_loop = [&](void) -> void
    {
        while (0 == pop_count)
        {
            auto iwdOperation = ioWorkerDeviceOperationQueue.Pop();
            if (nullptr != iwdOperation)
            {
                iwdOperation->SetDone();
                pop_count++;
            }
        }
    };
    std::thread t1(pop_loop), t2(pop_loop), t3(pop_loop);

    // When: Wait for try_lock fail case(Not guarantee, but almost trigger)
    usleep(1000); // 1ms
    ioWorkerDeviceOperationQueue.SubmitAndWait(IoWorkerDeviceOperationType::INSERT, nullptr);
    t1.join();
    t2.join();
    t3.join();

    // Then: Expect to load 1
    EXPECT_EQ(pop_count.load(), 1);
}

} // namespace pos

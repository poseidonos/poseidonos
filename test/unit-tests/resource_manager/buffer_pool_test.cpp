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

#include "src/resource_manager/buffer_pool.h"

#include <gtest/gtest.h>

#include "test/unit-tests/dpdk_wrapper/hugepage_allocator_mock.h"

using ::testing::Return;

namespace pos
{
TEST(BufferPool, GetBuffer_testGetBufferIfAllocatedSuccessfully)
{
    // Given
    char* mem = new char[4096];
    BufferInfo info;
    info.owner = "test";
    info.size = 4096; // 4KB
    info.count = 1;
    uint32_t socket = 0;
    MockHugepageAllocator* mockHugepageAllocator = new MockHugepageAllocator();
    EXPECT_CALL(*mockHugepageAllocator, AllocFromSocket).WillOnce(Return(mem));
    EXPECT_CALL(*mockHugepageAllocator, GetDefaultPageSize).WillRepeatedly(
        Return(2097152)); // 2MB
    EXPECT_CALL(*mockHugepageAllocator, Free).WillOnce([&mem](void* addr) {
            delete mem;
    });
    BufferPool* pool = new BufferPool(info, socket, mockHugepageAllocator);

    // When
    void* buffer = pool->TryGetBuffer();

    // Then
    EXPECT_EQ(mem, buffer);

    // Teardown
    delete pool;
    delete mockHugepageAllocator;
}

TEST(BufferPool, ReturnBuffer_testRepeatedlyGetandReturnBufferWhenBufferSizeIsTwo)
{
    // Given
    char* mem = new char[8192];
    void* buffer[2] = {mem, mem + 4096};
    BufferInfo info;
    info.owner = "test";
    info.size = 4096; // 4KB
    info.count = 2;
    uint32_t socket = 0;
    MockHugepageAllocator* mockHugepageAllocator = new MockHugepageAllocator();
    EXPECT_CALL(*mockHugepageAllocator, AllocFromSocket).WillOnce(Return(mem));
    EXPECT_CALL(*mockHugepageAllocator, GetDefaultPageSize).WillRepeatedly(
        Return(2097152)); // 2MB
    EXPECT_CALL(*mockHugepageAllocator, Free).WillOnce([&mem](void* addr) {
            delete mem;
    });
    BufferPool* pool = new BufferPool(info, socket, mockHugepageAllocator);
    int cycle = 5;

    // When
    for (int i = 0; i < cycle; i++)
    {
        void* firstBuffer = pool->TryGetBuffer();
        void* secondBuffer = pool->TryGetBuffer();

        // Then
        EXPECT_EQ(firstBuffer, buffer[0]);
        EXPECT_EQ(secondBuffer, buffer[1]);

        pool->ReturnBuffer(firstBuffer);
        pool->ReturnBuffer(secondBuffer);
    }

    // Teardown
    delete pool;
    delete mockHugepageAllocator;
}

} // namespace pos

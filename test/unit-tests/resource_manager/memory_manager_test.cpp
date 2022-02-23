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

#include "src/resource_manager/memory_manager.h"

#include <gtest/gtest.h>

#include "test/unit-tests/dpdk_wrapper/hugepage_allocator_mock.h"
#include "test/unit-tests/resource_manager/buffer_pool_factory_mock.h"
#include "test/unit-tests/resource_manager/buffer_pool_mock.h"
#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"

using ::testing::Return;
using testing::NiceMock;

namespace pos
{
TEST(MemoryManager, CreateBufferPool_testIfBufferPoolCreatedProperly)
{
    // Given
    BufferInfo info = {
        .owner = "test",
        .size = 65536, // 64KB
        .count = 1024};
    MockAffinityManager mockAffinityManager;
    EXPECT_CALL(mockAffinityManager, GetNumaIdFromCurrentThread).WillOnce(Return(0));
    MockBufferPool* mockBufferPool = new MockBufferPool(info, 0, nullptr);
    MockBufferPoolFactory* mockBufferPoolFactory = new MockBufferPoolFactory();
    EXPECT_CALL(*mockBufferPoolFactory, Create).WillOnce(Return(mockBufferPool));
    MemoryManager mm(mockBufferPoolFactory, &mockAffinityManager);

    // When
    BufferPool* pool = mm.CreateBufferPool(info);

    // Then
    EXPECT_EQ(pool, mockBufferPool);
}

TEST(MemoryManager, CreateBufferPool_testIfOwnerIsMissed)
{
    // Given
    BufferInfo info = {
        .owner = "",
        .size = 65536, // 64KB
        .count = 1024};
    NiceMock<MockBufferPoolFactory>* mockBufferPoolFactory = new NiceMock<MockBufferPoolFactory>;
    NiceMock<MockAffinityManager> mockAffinityManager;
    MemoryManager mm(mockBufferPoolFactory, &mockAffinityManager);

    // When
    BufferPool* pool = mm.CreateBufferPool(info);

    // Then
    EXPECT_EQ(pool, nullptr);
}

TEST(MemoryManager, CreateBufferPool_testIfSizeIsZero)
{
    // Given
    BufferInfo info = {
        .owner = "test",
        .size = 0, // 64KB
        .count = 1024};
    NiceMock<MockBufferPoolFactory>* mockBufferPoolFactory = new NiceMock<MockBufferPoolFactory>;
    NiceMock<MockAffinityManager> mockAffinityManager;
    MemoryManager mm(mockBufferPoolFactory, &mockAffinityManager);

    // When
    BufferPool* pool = mm.CreateBufferPool(info);

    // Then
    EXPECT_EQ(pool, nullptr);
}

TEST(MemoryManager, CreateBufferPool_testIfNonAlignedMemoryAllocationIsRejected)
{
    // Given
    BufferInfo info = {
        .owner = "test",
        .size = 64 * 1024 - 1,
        .count = 1024};
    NiceMock<MockBufferPoolFactory>* mockBufferPoolFactory = new NiceMock<MockBufferPoolFactory>;
    NiceMock<MockAffinityManager> mockAffinityManager;
    MemoryManager mm(mockBufferPoolFactory, &mockAffinityManager);

    // When
    BufferPool* pool = mm.CreateBufferPool(info);

    // Then
    EXPECT_EQ(pool, nullptr);
}

TEST(MemoryManager, CreateBufferPool_testIfSocketIsInvalid)
{
    // Given
    BufferInfo info = {
        .owner = "test",
        .size = 2 * 1024 * 1024, // 2MB
        .count = 1024};
    uint32_t socket = 100;
    MockAffinityManager mockAffinityManager;
    EXPECT_CALL(mockAffinityManager, GetNumaCount).WillOnce(Return(2));
    NiceMock<MockBufferPoolFactory>* mockBufferPoolFactory = new NiceMock<MockBufferPoolFactory>;
    MemoryManager mm(mockBufferPoolFactory, &mockAffinityManager);

    // When
    BufferPool* pool = mm.CreateBufferPool(info, socket);

    // Then
    EXPECT_EQ(pool, nullptr);
}

TEST(MemoryManager, DeleteBufferPool_testIfBufferPoolDeletedProperly)
{
    // Given
    BufferInfo info = {
        .owner = "test",
        .size = 65536, // 64KB
        .count = 1024};
    uint32_t socket = 0;
    MockBufferPool* mockBufferPool = new MockBufferPool(info, socket, nullptr);
    MockBufferPoolFactory* mockBufferPoolFactory = new MockBufferPoolFactory();
    EXPECT_CALL(*mockBufferPoolFactory, Create).WillOnce(Return(mockBufferPool));
    MemoryManager mm(mockBufferPoolFactory);
    BufferPool* pool = mm.CreateBufferPool(info);

    // When
    bool result = mm.DeleteBufferPool(pool);

    // Then
    EXPECT_EQ(result, true);
}

} // namespace pos

#include "src/spdk_wrapper/free_buffer_pool.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(FreeBufferPool, GetBuffer_BufferEmpty)
{
    // Given
    NiceMock<MockAffinityManager> mockAffinityManager;
    uint64_t maxBufferCount = 0;
    uint32_t bufferSize = 1024;
    void* expected = nullptr;
    EXPECT_CALL(mockAffinityManager, GetEventWorkerSocket()).WillOnce(Return(0));

    // When: Try to get buffer
    FreeBufferPool freeBufferPool(maxBufferCount, bufferSize, &mockAffinityManager);
    void* actual = freeBufferPool.GetBuffer();

    // Then: Buffer is empty
    EXPECT_EQ(expected, actual);
}

TEST(FreeBufferPool, ReturnBuffer_BufferIsNull)
{
    // Given
    NiceMock<MockAffinityManager> mockAffinityManager;
    uint64_t maxBufferCount = 0;
    uint32_t bufferSize = 1024;
    uint32_t socket = 0;
    void* buffer = nullptr;
    EXPECT_CALL(mockAffinityManager, GetEventWorkerSocket()).WillOnce(Return(socket));

    // When: Try to return null buffer
    FreeBufferPool freeBufferPool(maxBufferCount, bufferSize, &mockAffinityManager);
    freeBufferPool.ReturnBuffer(buffer);

    // Then: Do nothing
}

} // namespace pos

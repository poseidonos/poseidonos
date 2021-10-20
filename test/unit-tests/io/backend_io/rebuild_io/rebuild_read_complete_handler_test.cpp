#include "src/io/backend_io/rebuild_io/rebuild_read_complete_handler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/bio/ubio_mock.h"
#include "test/unit-tests/resource_manager/buffer_pool_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(RebuildReadCompleteHandler, RebuildReadCompleteHandler_Heap)
{
    // Given
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);

    // When : Create RebuildReadCompleteHandler object on heap
    RebuildReadCompleteHandler* rebuildReadCompleteHandler = new RebuildReadCompleteHandler(ubio, nullptr, 0, nullptr);

    // Then : Delete resource
    delete rebuildReadCompleteHandler;
}

TEST(RebuildReadCompleteHandler, RebuildReadCompleteHandler_Stack)
{
    // Given
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);

    // When : Create RebuildReadCompleteHandler object on stack
    RebuildReadCompleteHandler rebuildReadCompleteHandler(ubio, nullptr, 0, nullptr);

    // Then : Do nothing
}

TEST(RebuildReadCompleteHandler, RebuildReadCompleteHandler_DoSpecificJob_WithError)
{
    // Given
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);
    BufferInfo info =
        {
            .owner = "test",
            .size = 65536, // 64KB
            .count = 1024};
    uint32_t socket = 0;
    MockBufferPool* mockBufferPool = new MockBufferPool(info, socket, nullptr);

    // When : Create RebuildReadCompleteHandler and set error
    // Then : Check function calles
    ON_CALL(*ubio.get(), GetOriginUbio()).WillByDefault(Return(nullptr));
    EXPECT_CALL(*ubio.get(), GetOriginUbio()).Times(2);
    ON_CALL(*ubio, GetWholeBuffer()).WillByDefault(Return(nullptr));
    EXPECT_CALL(*ubio.get(), GetWholeBuffer()).Times(1);
    ON_CALL(*mockBufferPool, ReturnBuffer(_)).WillByDefault(Return());
    EXPECT_CALL(*mockBufferPool, ReturnBuffer(_)).Times(1);
    RebuildReadCompleteHandler rebuildReadCompleteHandler(ubio, nullptr, 0, mockBufferPool);
    rebuildReadCompleteHandler.InformError(IOErrorType::DEVICE_ERROR);
    rebuildReadCompleteHandler.Execute();

    delete mockBufferPool;
}

} // namespace pos

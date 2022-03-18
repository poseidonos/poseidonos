#include "src/allocator/wbstripe_manager/write_stripe_completion.h"

#include "test/unit-tests/resource_manager/buffer_pool_mock.h"
#include "test/unit-tests/allocator/wbstripe_manager/stripe_load_status_mock.h"

#include <gtest/gtest.h>

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(WriteStripeCompletion, _DoSpecificJob_)
{
    BufferInfo info;
    NiceMock<MockBufferPool> bufferPool(info, 0, nullptr);
    NiceMock<MockStripeLoadStatus> status;

    std::vector<void*> buffers;
    buffers.push_back(malloc(100));
    buffers.push_back(malloc(100));
    buffers.push_back(malloc(100));

    WriteStripeCompletion writeStripeCompletion(&bufferPool, buffers, &status);

    EXPECT_CALL(status, StripeLoaded).Times(1);
    EXPECT_CALL(bufferPool, ReturnBuffer).Times(3);

    bool result = writeStripeCompletion.Execute();
    EXPECT_EQ(result, true);

    for (auto b : buffers)
    {
        free(b);
    }
}

} // namespace pos

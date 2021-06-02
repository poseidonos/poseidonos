#include "src/io/backend_io/flush_submission.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <list>

#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator_service/allocator_service.h"
#include "src/include/backend_event.h"
#include "src/include/meta_const.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/backend_io/stripe_map_update_request.h"
#include "src/logger/logger.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator/wb_stripe_manager/stripe_mock.h"
#include "test/unit-tests/io_submit_interface/i_io_submit_handler_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(FlushSubmission, FlushSubmission_Constructor_OneArgument_Stack)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    std::string arr_name = "";

    // When : Creat flushSubmission
    NiceMock<MockIIOSubmitHandler> mockIIOSubmitHandler;
    IIOSubmitHandler::RegisterInstance(&mockIIOSubmitHandler);
    FlushSubmission flushSubmission(&mockStripe, arr_name);

    // Then : Do nothing
}

TEST(FlushSubmission, FlushSubmission_Constructor_OneArgument_Heap)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    std::string arr_name = "";

    // When : Creat flushSubmission
    NiceMock<MockIIOSubmitHandler> mockIIOSubmitHandler;
    IIOSubmitHandler::RegisterInstance(&mockIIOSubmitHandler);
    FlushSubmission* flushSubmission = new FlushSubmission(&mockStripe, arr_name);

    // Then : Release memory
    delete flushSubmission;
}

TEST(FlushSubmission, FlushSubmission_Constructor_ThreeArguments)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIIOSubmitHandler> mockIIOSubmitHandler;
    std::string arr_name = "";

    // When : constructor is called
    FlushSubmission flushSubmission(&mockStripe, &mockIWBStripeAllocator, &mockIIOSubmitHandler, arr_name, nullptr);

    // Then : Do nothing
}

TEST(FlushSubmission, FlushSubmission_Execute_CheckBufferSizeAndReturn)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIIOSubmitHandler> mockIIOSubmitHandler;
    std::string arr_name = "";
    FlushSubmission flushSubmission(&mockStripe, &mockIWBStripeAllocator, &mockIIOSubmitHandler, arr_name, nullptr);
    const uint32_t BUFFER_SIZE = 4;
    uint32_t testBuffer[BUFFER_SIZE];
    uint32_t actualBufferSize;
    bool actualReturn;

    // When 1: Add buffer and Execute() called
    std::vector<void*> dataBuffer;
    for (uint32_t i = 0; i < BUFFER_SIZE; ++i)
    {
        dataBuffer.push_back(static_cast<void*>(&testBuffer[i]));
    }
    ON_CALL(mockIWBStripeAllocator, AllocateUserDataStripeId(_)).WillByDefault(Return(0));
    ON_CALL(mockIIOSubmitHandler, SubmitAsyncIO(_, _, _, _, _, _, _)).WillByDefault(Return(IOSubmitHandlerStatus::SUCCESS));
    ON_CALL(mockStripe, DataBufferBegin()).WillByDefault(Return(static_cast<std::vector<void*>::iterator>(dataBuffer.begin())));
    ON_CALL(mockStripe, DataBufferEnd()).WillByDefault(Return(static_cast<std::vector<void*>::iterator>(dataBuffer.end())));
    flushSubmission.Execute();

    // Then 1: bufferList size equals bufferSize
    actualBufferSize = flushSubmission.GetBufferListSize();
    ASSERT_EQ(actualBufferSize, BUFFER_SIZE);
}

TEST(FlushSubmission, FlushSubmission_Execute_CheckReturnValue)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIIOSubmitHandler> mockIIOSubmitHandler;
    std::string arr_name = "";
    FlushSubmission flushSubmission(&mockStripe, &mockIWBStripeAllocator, &mockIIOSubmitHandler, arr_name, nullptr);
    bool actualReturn;

    // When 1: Add SubmitAsumcIO return SUCCESS
    ON_CALL(mockIIOSubmitHandler, SubmitAsyncIO(_, _, _, _, _, _, _)).WillByDefault(Return(IOSubmitHandlerStatus::SUCCESS));
    actualReturn = flushSubmission.Execute();

    // Then 1: Return true
    ASSERT_EQ(actualReturn, true);

    // When 2: Add SubmitAsumcIO return FAIL_IN_SYSTEM_STOP
    ON_CALL(mockIIOSubmitHandler, SubmitAsyncIO(_, _, _, _, _, _, _)).WillByDefault(Return(IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP));
    actualReturn = flushSubmission.Execute();

    // Then 2: Return true
    ASSERT_EQ(actualReturn, true);

    // When 3: Add SubmitAsumcIO return FAIL
    ON_CALL(mockIIOSubmitHandler, SubmitAsyncIO(_, _, _, _, _, _, _)).WillByDefault(Return(IOSubmitHandlerStatus::FAIL));
    actualReturn = flushSubmission.Execute();

    // Then 3: Return false
    ASSERT_EQ(actualReturn, false);

    // When 4: Add SubmitAsumcIO return TRYLOCK_FAIL
    ON_CALL(mockIIOSubmitHandler, SubmitAsyncIO(_, _, _, _, _, _, _)).WillByDefault(Return(IOSubmitHandlerStatus::TRYLOCK_FAIL));
    actualReturn = flushSubmission.Execute();

    // Then 4: Return false
    ASSERT_EQ(actualReturn, false);
}

TEST(FlushSubmission, FlushSubmission_GetBufferListSize_EmptyArgument)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    uint32_t bufferSize;
    std::string arr_name = "";

    // When : Creat flushSubmit and call GetBufferListSize
    FlushSubmission flushSubmission(&mockStripe, arr_name);
    bufferSize = flushSubmission.GetBufferListSize();

    // Then : GetBufferListSize return 0
    ASSERT_EQ(bufferSize, 0);
}

} // namespace pos

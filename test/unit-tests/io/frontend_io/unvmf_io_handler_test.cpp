#include "src/io/frontend_io/unvmf_io_handler.h"

#include <gtest/gtest.h>
#include <exception>
#include "spdk/ibof.h"
#include "src/include/pos_event_id.hpp"
#include "src/include/branch_prediction.h"
#include "src/io/frontend_io/aio.h"
#include "src/bio/ubio.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/spdk_wrapper/event_framework_api.h"
#if defined QOS_ENABLED_FE
#include "src/qos/qos_manager.h"
#include "src/io/frontend_io/aio_submission_adapter.h"
#endif

namespace pos
{
TEST(UNVMfIOHandler, UNVMfCompleteHandler_Normal)
{
    // Given

    // When
    UNVMfCompleteHandler();

    // Then: Do Nothing
}

TEST(UNVMfIOHandler, UNVMfSubmitHandler_ibofIoNullptr)
{
    // Given: ibof_io is nullptr
    struct ibof_io *io = nullptr;
    int actual, expected;

    // When: Submit ibof_io 
    actual = UNVMfSubmitHandler(io);
    
    // Then: UNVMfSubmitHandler throws exception and return succeess
    expected = IBOF_IO_STATUS_SUCCESS;
    ASSERT_EQ(actual, expected);
}

TEST(UNVMfIOHandler, UNVMfSubmitHandler_InvalidIOType)
{
    // Given
    struct ibof_io io = {4, 1, nullptr, 1, 0, 0, nullptr, nullptr};
    int actual, expected;
    
    // When
    actual = UNVMfSubmitHandler(&io);

    // Then: UNVMfSubmitHandler throws exception and return success
    expected = IBOF_IO_STATUS_SUCCESS;
    ASSERT_EQ(actual, expected);
}

TEST(UNVMfIOHandler, UNVMfSubmitHandler_WrongBuffer)
{
    // Given
    struct ibof_io io = {IO_TYPE::READ, 1, nullptr, 0, 0, 0, nullptr, nullptr};
    int actual, expected;

    // When
    actual = UNVMfSubmitHandler(&io);

    // Then: UNVMfSubmitHandler throws exception and return success
    expected = IBOF_IO_STATUS_SUCCESS;
    ASSERT_EQ(actual, expected);
}
}
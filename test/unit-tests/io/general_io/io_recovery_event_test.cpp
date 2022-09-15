#include "src/io/general_io/io_recovery_event.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/array/service/array_service_layer.h"
#include "src/array/service/io_device_checker/i_io_device_checker.h"
#include "src/bio/ubio.h"
#include "src/event_scheduler/io_completer.h"
#include "src/include/branch_prediction.h"
#include "src/io/backend_io/rebuild_io/rebuild_read.h"
#include "src/include/backend_event.h"
#include "test/unit-tests/event_scheduler/io_completer_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(IoRecoveryEvent, IoRecoveryEvent_Stack)
{
    // Given: Minimum ubio setting for IoRecoveryEvent's parameter
    int buf[10];
    UbioSmartPtr ubio = std::make_shared<Ubio>((void*)buf, 10, 0);

    // When: Try to create a local IoRecoveryEvent object with 1 argument
    IoRecoveryEvent ioRecoveryEvent{ubio};

    // Then: Do nothing
}

TEST(IoRecoveryEvent, IoRecoveryEvent_Heap)
{
    // Given: Minimum ubio setting for IoRecoveryEvent's parameter
    int buf[10];
    UbioSmartPtr ubio = std::make_shared<Ubio>((void*)buf, 10, 0);

    // When: Try to create a dynamic IoRecoveryEvent object with 1 argument
    IoRecoveryEvent* ioRecoveryEvent = new IoRecoveryEvent{ubio};

    // Then: Release memory
    delete ioRecoveryEvent;
}

TEST(IoRecoveryEvent, Execute_ReadWithoutRecovery)
{
    // Given: ubio setting for read type testing & IoCompleter mocking
    int buf[10];
    UbioSmartPtr ubio = std::make_shared<Ubio>((void*)buf, 10, 0);
    ubio->dir = UbioDir::Read;
    ubio->SetError(IOErrorType::GENERIC_ERROR);
    NiceMock<MockIoCompleter> mockIoCompleter{ubio};
    IoRecoveryEvent ioRecoveryEvent{ubio, &mockIoCompleter};

    ON_CALL(mockIoCompleter, CompleteUbioWithoutRecovery(_,_)).WillByDefault(Return());

    bool actual, expected{true};

    // When: Try to call Execute method
    actual = ioRecoveryEvent.Execute();

    // Then: return value(actual) should be true, ubio error type should be reset
    ASSERT_EQ(actual, expected);
    ASSERT_EQ(ubio->GetError(), IOErrorType::SUCCESS);
}

TEST(IoRecoveryEvent, Execute_WriteWithoutRecovery)
{
    // Given: ubio setting for write type testing & IoCompleter mocking
    int buf[10];
    UbioSmartPtr ubio = std::make_shared<Ubio>((void*)buf, 10, 0);
    ubio->dir = UbioDir::Write;
    ubio->SetError(IOErrorType::TRANSPORT_FAIL);
    NiceMock<MockIoCompleter> mockIoCompleter{ubio};
    IoRecoveryEvent ioRecoveryEvent{ubio, &mockIoCompleter};

    ON_CALL(mockIoCompleter, CompleteUbioWithoutRecovery(_,_)).WillByDefault(Return());

    bool actual, expected{true};

    // When: Try to call Execute method
    actual = ioRecoveryEvent.Execute();

    // Then: return value(actual) should be true, ubio error type should not be reset
    ASSERT_EQ(actual, expected);
}

TEST(IoRecoveryEvent, Execute_WriteUncorWithoutRecovery)
{
    // Given: ubio setting for write uncor type testing & IoCompleter mocking
    int buf[10];
    UbioSmartPtr ubio = std::make_shared<Ubio>((void*)buf, 10, 0);
    ubio->dir = UbioDir::WriteUncor;
    ubio->SetError(IOErrorType::DEVICE_ERROR);
    NiceMock<MockIoCompleter> mockIoCompleter{ubio};
    IoRecoveryEvent ioRecoveryEvent{ubio, &mockIoCompleter};

    ON_CALL(mockIoCompleter, CompleteUbioWithoutRecovery(_,_)).WillByDefault(Return());

    bool actual, expected{true};

    // When: Try to call Execute method
    actual = ioRecoveryEvent.Execute();

    // Then: return value(actual) should be true, ubio error type should not be reset
    ASSERT_EQ(actual, expected);
    ASSERT_EQ(ubio->GetError(), IOErrorType::DEVICE_ERROR);
}

} // namespace pos

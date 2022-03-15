#include "src/array/service/array_service_layer.h"

#include <gtest/gtest.h>

#include "test/unit-tests/array/service/io_device_checker/i_device_checker_mock.h"
#include "test/unit-tests/array/service/io_recover/i_recover_mock.h"
#include "test/unit-tests/array/service/io_translator/i_translator_mock.h"
#include "src/include/pos_event_id.h"

namespace pos
{
TEST(ArrayServiceLayer, ArrayServiceLayer_testConstructor)
{
    // Given
    // When
    ArrayServiceLayer arrayServiceLayer;
    // Then
}

TEST(ArrayServiceLayer, Getter_)
{
}

TEST(ArrayServiceLayer, Setter_)
{
}

TEST(ArrayServiceLayer, Register_testIfRegisterNullCheckerFailure)
{
    // Given
    ArrayServiceLayer arrayServiceLayer;
    string mockArrayName = "mockArray";
    unsigned int mockArrayIndex = 0;
    ArrayTranslator trans;
    ArrayRecover recover;
    MockIDeviceChecker* checker = nullptr;
    // When
    int actual = arrayServiceLayer.Register(mockArrayName, mockArrayIndex, trans, recover, checker);
    // Then
    ASSERT_EQ(EID(MOUNT_ARRAY_UNABLE_TO_REGISTER_DEVICECHECKER), actual);
}

TEST(ArrayServiceLayer, Unregister_testIfUnregisterEmptyServices)
{
    // Given
    ArrayServiceLayer arrayServiceLayer;
    string mockArrayName = "mockArray";
    unsigned int mockArrayIndex = 0;
    // When
    arrayServiceLayer.Unregister(mockArrayName, mockArrayIndex);
    // Then
}

TEST(ArrayServiceLayer, GetTranslator_)
{
}

TEST(ArrayServiceLayer, GetRecover_)
{
}

TEST(ArrayServiceLayer, GetDeviceChecker_)
{
}

} // namespace pos

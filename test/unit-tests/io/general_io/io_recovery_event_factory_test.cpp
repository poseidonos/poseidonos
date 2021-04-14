#include "src/io/general_io/io_recovery_event_factory.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(IoRecoveryEventFactory, IoRecoveryEventFactory_Create)
{
    // Givne:
    IoRecoveryEventFactory ioRecoveryEventFactory;
    UbioSmartPtr ubio;

    // When: Call Create with ubio
    auto ret = ioRecoveryEventFactory.Create(ubio);

    // Then: Chech return value
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(typeid(ret), typeid(EventSmartPtr));
}

} // namespace pos

#include "src/singleton_info/singleton_info.h"
#include "src/qos/qos_spdk_manager.h"
#include <gtest/gtest.h>

namespace pos
{
TEST(SingletonInfo, SingletonInfo)
{
    // Given : new debug Info object is constructed.
    SingletonInfo* localsingletonInfo = new SingletonInfo;
    // When : Nothing
    // Then : delete.
    delete localsingletonInfo;
}

TEST(SingletonInfo, Update)
{
}

} // namespace pos

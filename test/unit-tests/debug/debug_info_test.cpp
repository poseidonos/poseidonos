#include "src/debug/debug_info.h"
#include "src/qos/qos_spdk_manager.h"
#include <gtest/gtest.h>

namespace pos
{
TEST(DebugInfo, DebugInfo)
{
    // Given : new debug Info object is constructed.
    DebugInfo* localDebugInfo = new DebugInfo;
    // When : Nothing
    // Then : delete.
    delete localDebugInfo;
}

TEST(DebugInfo, Update)
{
}

} // namespace pos

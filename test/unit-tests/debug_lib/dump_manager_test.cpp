#include "src/debug_lib/dump_manager.h"
#include "src/debug_lib/debug_info_queue.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(DumpManager, Constructor)
{
    DumpManager dumpManager;
}

TEST(DumpManager, RegisterDump)
{
    DumpManager dumpManager;
    DebugInfoQueue<int> dumpModule("test5", 30, false);
    dumpManager.RegisterDump("test5", &dumpModule);
}

TEST(DumpManager, SetEnableModuleByCLI)
{
    DumpManager dumpManager;
    DebugInfoQueue<int> dumpModule("test5", 30, false);
    dumpManager.RegisterDump("test5", &dumpModule);
    dumpManager.SetEnableModuleByCLI("test5", true);
    EXPECT_EQ(dumpModule.IsEnable(), true);
}

} // namespace pos

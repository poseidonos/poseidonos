#include "src/dump/dump_manager.h"
#include "src/dump/dump_module.h"

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
    DumpModule<int> dumpModule("test5", 30, false);
    dumpManager.RegisterDump("test5", &dumpModule);
}

TEST(DumpManager, SetEnableModuleByCLI)
{
    DumpManager dumpManager;
    DumpModule<int> dumpModule("test5", 30, false);
    dumpManager.RegisterDump("test5", &dumpModule);
    dumpManager.SetEnableModuleByCLI("test5", true);
    EXPECT_EQ(dumpModule.IsEnable(), true);
}

} // namespace pos

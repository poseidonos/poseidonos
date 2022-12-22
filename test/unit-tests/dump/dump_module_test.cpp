#include "src/debug_lib/debug_info_queue.h"
#include "src/debug_lib/debug_info_queue.hpp"

#include <gtest/gtest.h>

namespace pos
{

TEST(DumpObject, DumpObject_Constructor)
{
    // Without argument
    DumpObject<int>* dumpObject;
    dumpObject = new DumpObject<int>();
    delete dumpObject;
    // With 2 arguments
    int a = 3;
    dumpObject = new DumpObject<int>(a, 0);
    delete dumpObject;
}

} // namespace pos

namespace pos
{
TEST(DebugInfoQueue, DebugInfoQueue)
{
    DebugInfoQueue<int>* dumpModule;
    // Without Argument
    dumpModule = new DebugInfoQueue<int>();
    delete dumpModule;
    dumpModule = new DebugInfoQueue<int>("test1", 30, true);
    delete dumpModule;

}

TEST(DebugInfoQueue, AddDebugInfo)
{
    DebugInfoQueue<int>* dumpModule;
    dumpModule = new DebugInfoQueue<int>("test2", 30, true);
    int a = 3;
    dumpModule->AddDebugInfo(a, 0);
    a = 8;
    // Without lock
    dumpModule->AddDebugInfo(a, 0, false);

    delete dumpModule;
}

TEST(DebugInfoQueue, SetEnable)
{
    DebugInfoQueue<int>* dumpModule;
    dumpModule = new DebugInfoQueue<int>("test3", 30, false);
    int a = 8;
    dumpModule->AddDebugInfo(a, 0, false);
    dumpModule->SetEnable(true);
    dumpModule->AddDebugInfo(a, 0, false);
    EXPECT_EQ(dumpModule->IsEnable(), true);

    delete dumpModule;
}

TEST(DebugInfoQueue, GetPoolSize)
{
    DebugInfoQueue<int>* dumpModule;
    dumpModule = new DebugInfoQueue<int>("test4", 30, false);
    uint32_t poolSize = dumpModule->GetPoolSize();
    EXPECT_GT(poolSize, 0);
    delete dumpModule;
}

} // namespace pos

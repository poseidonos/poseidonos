#include "src/dump/dump_module.h"
#include "src/dump/dump_module.hpp"

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
TEST(DumpModule, DumpModule)
{
    DumpModule<int>* dumpModule;
    // Without Argument
    dumpModule = new DumpModule<int>();
    delete dumpModule;
    dumpModule = new DumpModule<int>("test1", 30, true);
    delete dumpModule;

}

TEST(DumpModule, AddDump)
{
    DumpModule<int>* dumpModule;
    dumpModule = new DumpModule<int>("test2", 30, true);
    int a = 3;
    dumpModule->AddDump(a, 0);
    a = 8;
    // Without lock
    dumpModule->AddDump(a, 0, false);

    delete dumpModule;
}

TEST(DumpModule, SetEnable)
{
    DumpModule<int>* dumpModule;
    dumpModule = new DumpModule<int>("test3", 30, false);
    int a = 8;
    dumpModule->AddDump(a, 0, false);
    dumpModule->SetEnable(true);
    dumpModule->AddDump(a, 0, false);
    EXPECT_EQ(dumpModule->IsEnable(), true);

    delete dumpModule;
}

TEST(DumpModule, GetPoolSize)
{
    DumpModule<int>* dumpModule;
    dumpModule = new DumpModule<int>("test4", 30, false);
    uint32_t poolSize = dumpModule->GetPoolSize();
    EXPECT_GT(poolSize, 0);
    delete dumpModule;
}

} // namespace pos

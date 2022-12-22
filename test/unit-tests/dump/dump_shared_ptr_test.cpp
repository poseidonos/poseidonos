#include "src/debug_lib/dump_shared_ptr.h"
#include "src/debug_lib/dump_shared_ptr.hpp"

#include <gtest/gtest.h>

namespace pos
{
class DumpSharedPtrTest : public DumpSharedPtr<DumpSharedPtrTest*, 3>
{
public:
    int i;
    int j;
};

TEST(DumpSharedPtr, newdeleteOperator)
{
    // Given : DumpSharedPtr is turned on
    pos::DumpSharedModuleInstanceEnable::debugLevelEnable = true;
    // When : DumpSharedPtr is newly created
    DumpSharedPtrTest* dumpSharedPtr1 = new DumpSharedPtrTest;
    DumpSharedPtrTest* dumpSharedPtr2 = new DumpSharedPtrTest;
    // Then : Delete DumpSharedPtr
    delete dumpSharedPtr1;
    delete dumpSharedPtr2;
    DumpSharedPtrTest* dumpSharedPtr3 = new DumpSharedPtrTest[10];
    delete[] dumpSharedPtr3;
}

} // namespace pos

namespace pos
{

TEST(DumpSharedModule, DumpSharedModule_Complex)
{
    // Given : DumpSharedPtr is turned on, and Reset instance
    pos::DumpSharedModuleInstanceEnable::debugLevelEnable = true;
    DumpSharedModuleInstanceSingleton<DumpSharedPtrTest*, 3>::ResetInstance();
    // When : Dump shared ptr is created
    DumpSharedPtrTest* dumpSharedPtr1 = new DumpSharedPtrTest;
    // Then : Dump shared ptr is deleted
    delete dumpSharedPtr1;

    // Given : DumpSharedPtr is turned on, and Reset instance
    DumpSharedModuleInstanceSingleton<DumpSharedPtrTest*, 3>::ResetInstance();
    pos::DumpSharedModuleInstanceEnable::debugLevelEnable = true;
    // When : Debug option is on and Dump sharedPtr is created, and Debug option is off again
    dumpSharedPtr1 = new DumpSharedPtrTest;
    pos::DumpSharedModuleInstanceEnable::debugLevelEnable = false;
    // Then : Delete dump SharedPtr
    delete dumpSharedPtr1;
    DumpSharedModuleInstanceSingleton<DumpSharedPtrTest*, 3>::ResetInstance();

    // Given : DumpSharedPtr is turned on, and Reset instance
    int ptr = 5;
    // Then : DumpSharedPtr is turned on, and use Add / Delete function
    DumpSharedModuleInstanceSingleton<int*, 2>::Instance()->DumpInstance()->Add(&ptr);
    DumpSharedModuleInstanceSingleton<int*, 2>::Instance()->DumpInstance()->Delete(&ptr);
    pos::DumpSharedModuleInstanceEnable::debugLevelEnable = true;
    // Then : DumpSharedPtr is turned on, and use Add / Delete function without lock
    DumpSharedModuleInstanceSingleton<int*, 2>::Instance()->DumpInstance()->Add(&ptr, false);
    DumpSharedModuleInstanceSingleton<int*, 2>::Instance()->DumpInstance()->Delete(&ptr, false);
}

} // namespace pos

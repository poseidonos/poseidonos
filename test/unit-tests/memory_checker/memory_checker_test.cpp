#include "src/memory_checker/memory_checker.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MemoryChecker, New)
{
    // Given, When : Memory Checker is enabled
    MemoryChecker::Enable(true);
    // Then : New Memory is allocated with size 20
    void *ptr = MemoryChecker::New(20);
    free(ptr);
    MemoryChecker::Enable(false);
}

TEST(MemoryChecker, Delete)
{
    // Given : Memory Checker is enabled
    MemoryChecker::Enable(true);
    // When : New Memory is allocated with size 20
    void *ptr = MemoryChecker::New(20);
    // Then : Use Delete function for ptr
    MemoryChecker::Delete(ptr);
    MemoryChecker::Enable(false);
}

class DummyMemoryCheckerTestClass
{
private:
    int i;
    int j;
};

TEST(MemoryChecker, Operator_Overloading)
{
    // Given, When : new int pointer is created
    int* ptr = new int;
    // Then : delete ptr
    delete ptr;
    // Given, When : new array int pointer is created
    ptr = new int[5];
    // Then : delete array ptr
    delete [] ptr;
    // Given, When : new class pointer is created
    DummyMemoryCheckerTestClass *dummyPtr;
    dummyPtr = new DummyMemoryCheckerTestClass;
    // Then : delete ptr
    delete dummyPtr;
    // Given, When : new class pointer is created
    dummyPtr = new DummyMemoryCheckerTestClass[5];
    // Then : delete array ptr
    delete [] dummyPtr;
}


TEST(MemoryChecker, EnableStackTrace)
{
    void *inputDumpStack[FreeListInfo::MAX_DUMP_STACK_COUNT] = {0,};
    // Given : StackTrace is on
    MemoryChecker::EnableStackTrace(true);
    // When : Free list is initialized
    FreeListInfo freeListInfo(3);
    // Then : New free list is constructed based on input dump stack and print Dump stack
    FreeListInfo freeListInfo2(3, inputDumpStack);
    freeListInfo2.PrintDumpStack();
    
    // Given : StackTrace is off
    MemoryChecker::EnableStackTrace(false);
    // When : Free list is initialized
    FreeListInfo freeListInfo3(3);
    // Then : New free list is constructed based on input dump stack and print Dump stack
    FreeListInfo freeListInfo4(3, inputDumpStack);
    freeListInfo4.PrintDumpStack();
}

TEST(MemoryChecker, CheckDoubleFree)
{
    // Given : Enabling as true
    MemoryChecker::Enable(true);
    // When : New int pointer created and deleted
    int* ptr = new int;
    delete ptr;
    // Then : delete again, and check if assert is triggered or not
    EXPECT_DEATH(MemoryChecker::Delete(ptr), "");
}

TEST(MemoryChecker, EraseFromFreeList)
{
    // Given : Enabling as true
    MemoryChecker::Enable(true);
    // When : New void pointer created, Erase From free list
    int* ptr = (int *)MemoryChecker::New(20);
    delete ptr;
    // Then : It should not be recursive call
    MemoryChecker::Enable(false);
    MemoryChecker::EraseFromFreeList(reinterpret_cast<uint64_t>(ptr), 8);
    MemoryChecker::EraseFromFreeList(reinterpret_cast<uint64_t>(ptr) + 8, 20);
}

TEST(MemoryChecker, InvalidAccess)
{
    // Given : Enabling as true
    MemoryChecker::Enable(true);
    // When : New void pointer created, and 8byte later, invalid access is incurred
    uint64_t* ptr = new uint64_t;
    *(ptr + 1) = 0x0;
    // Then : check if delete function incurs assert
    EXPECT_DEATH(MemoryChecker::Delete(ptr), "");
    MemoryChecker::Enable(false);
}

} // namespace pos

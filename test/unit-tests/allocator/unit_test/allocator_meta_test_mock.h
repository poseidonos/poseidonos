#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/unit_test/allocator_meta_test.h"

class MockFlushDoneEvent : public FlushDoneEvent
{
public:
    using FlushDoneEvent::FlushDoneEvent;
    MOCK_METHOD(bool, Execute, (), (override));
};

class MockAllocatorMetaTest : public AllocatorMetaTest
{
public:
    using AllocatorMetaTest::AllocatorMetaTest;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

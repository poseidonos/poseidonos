#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/meta_file_intf/meta_file_intf_test.h"

class MockMetaFileTest : public MetaFileTest
{
public:
    using MetaFileTest::MetaFileTest;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

class MockMockFileTest : public MockFileTest
{
public:
    using MockFileTest::MockFileTest;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

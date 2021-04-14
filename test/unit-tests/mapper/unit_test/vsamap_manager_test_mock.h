#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/unit_test/vsamap_manager_test.h"

namespace pos
{
class MockVSAMapManagerTest : public VSAMapManagerTest
{
public:
    using VSAMapManagerTest::VSAMapManagerTest;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

class MockAccessRequest : public AccessRequest
{
public:
    using AccessRequest::AccessRequest;
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/unit_test/reverse_map_test.h"

namespace pos
{
class MockReverseMapTest : public ReverseMapTest
{
public:
    using ReverseMapTest::ReverseMapTest;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

} // namespace pos

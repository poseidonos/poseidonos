#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/unit_test/sequential_page_finder_test.h"

namespace pos
{
class MockSequentialPageFinderTest : public SequentialPageFinderTest
{
public:
    using SequentialPageFinderTest::SequentialPageFinderTest;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

} // namespace pos

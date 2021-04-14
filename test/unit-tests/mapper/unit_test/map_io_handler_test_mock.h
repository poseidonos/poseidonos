#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/unit_test/map_io_handler_test.h"

namespace pos
{
class MockMapIoHandlerTest : public MapIoHandlerTest
{
public:
    using MapIoHandlerTest::MapIoHandlerTest;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

} // namespace pos

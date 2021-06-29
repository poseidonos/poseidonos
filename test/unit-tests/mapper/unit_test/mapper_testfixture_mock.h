#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/unit_test/mapper_testfixture.h"

namespace pos
{
class MockMapperTestFixture : public MapperTestFixture
{
public:
    using MapperTestFixture::MapperTestFixture;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

} // namespace pos

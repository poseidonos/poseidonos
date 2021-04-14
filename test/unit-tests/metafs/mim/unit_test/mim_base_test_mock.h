#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/unit_test/mim_base_test.h"

namespace pos
{
class MockUtMIMBasic : public UtMIMBasic
{
public:
    using UtMIMBasic::UtMIMBasic;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

} // namespace pos

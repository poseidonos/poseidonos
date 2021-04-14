#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/unit_test/mvm_base_test.h"

namespace pos
{
class MockUtMVMBasic : public UtMVMBasic
{
public:
    using UtMVMBasic::UtMVMBasic;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

} // namespace pos

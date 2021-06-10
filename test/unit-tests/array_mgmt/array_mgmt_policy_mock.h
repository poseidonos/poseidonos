#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/include/array_mgmt_policy.h"

namespace pos
{
class MockArrayMgmtPolicy : public ArrayMgmtPolicy
{
public:
    using ArrayMgmtPolicy::ArrayMgmtPolicy;
};

} // namespace pos

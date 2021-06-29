#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/array_name_policy.h"

namespace pos
{
class MockArrayNamePolicy : public ArrayNamePolicy
{
public:
    using ArrayNamePolicy::ArrayNamePolicy;
};

} // namespace pos

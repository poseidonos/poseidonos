#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/resource_array.h"

namespace pos
{
class MockResourceArray : public ResourceArray
{
public:
    using ResourceArray::ResourceArray;
};

} // namespace pos

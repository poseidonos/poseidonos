#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/resource_nvram.h"

namespace pos
{
class MockResourceNvramStripes : public ResourceNvramStripes
{
public:
    using ResourceNvramStripes::ResourceNvramStripes;
};

} // namespace pos

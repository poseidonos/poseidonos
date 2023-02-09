#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/pbr.h"

namespace pbr
{
class MockPbr : public Pbr
{
public:
    using Pbr::Pbr;
};

} // namespace pbr

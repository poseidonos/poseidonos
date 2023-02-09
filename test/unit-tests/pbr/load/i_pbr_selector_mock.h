#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/load/i_pbr_selector.h"

namespace pbr
{
class MockIPbrSelector : public IPbrSelector
{
public:
    using IPbrSelector::IPbrSelector;
    MOCK_METHOD(int, Select, (vector<AteData*>& candidates), (override));
};

} // namespace pbr

#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/load/pbr_selector.h"

namespace pbr
{
class MockPbrSelector : public PbrSelector
{
public:
    using PbrSelector::PbrSelector;
    MOCK_METHOD(int, Select, (vector<AteData*>& candidates), (override));
};

} // namespace pbr

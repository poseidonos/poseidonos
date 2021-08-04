#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/gc/flow_control/state_distributer.h"

namespace pos
{
class MockStateDistributer : public StateDistributer
{
public:
    using StateDistributer::StateDistributer;
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD((std::tuple<uint32_t, uint32_t>), Distribute, (uint32_t freeSegments), (override));
};

} // namespace pos

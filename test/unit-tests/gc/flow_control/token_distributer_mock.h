#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/gc/flow_control/token_distributer.h"

namespace pos
{
class MockTokenDistributer : public TokenDistributer
{
public:
    using TokenDistributer::TokenDistributer;
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD((std::tuple<uint32_t, uint32_t>), Distribute, (uint32_t freeSegments), (override));
};

} // namespace pos

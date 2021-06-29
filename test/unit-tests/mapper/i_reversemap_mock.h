#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/i_reversemap.h"

namespace pos
{
class MockIReverseMap : public IReverseMap
{
public:
    using IReverseMap::IReverseMap;
    MOCK_METHOD(ReverseMapPack*, GetReverseMapPack, (StripeId wbLsid), (override));
    MOCK_METHOD(ReverseMapPack*, AllocReverseMapPack, (bool gcDest), (override));
};

} // namespace pos

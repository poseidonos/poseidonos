#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/metadata/stripe_map_update.h"

namespace pos
{
class MockStripeMapUpdate : public StripeMapUpdate
{
public:
    using StripeMapUpdate::StripeMapUpdate;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

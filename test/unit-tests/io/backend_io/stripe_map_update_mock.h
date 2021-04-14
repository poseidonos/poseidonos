#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/backend_io/stripe_map_update.h"

namespace pos
{
class MockStripeMapUpdate : public StripeMapUpdate
{
public:
    using StripeMapUpdate::StripeMapUpdate;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

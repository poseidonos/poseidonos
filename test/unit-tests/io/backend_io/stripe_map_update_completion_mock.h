#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/backend_io/stripe_map_update_completion.h"

namespace pos
{
class MockStripeMapUpdateCompletion : public StripeMapUpdateCompletion
{
public:
    using StripeMapUpdateCompletion::StripeMapUpdateCompletion;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

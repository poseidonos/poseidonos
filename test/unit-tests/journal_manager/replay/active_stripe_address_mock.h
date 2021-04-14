#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/active_stripe_address.h"

namespace pos
{
class MockActiveStripeAddr : public ActiveStripeAddr
{
public:
    using ActiveStripeAddr::ActiveStripeAddr;
};

} // namespace pos

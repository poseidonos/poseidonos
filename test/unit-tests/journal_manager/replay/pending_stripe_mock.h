#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/pending_stripe.h"

namespace pos
{
class MockPendingStripe : public PendingStripe
{
public:
    using PendingStripe::PendingStripe;
};

} // namespace pos

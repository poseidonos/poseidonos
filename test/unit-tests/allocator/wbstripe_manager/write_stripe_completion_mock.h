#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/wbstripe_manager/write_stripe_completion.h"

namespace pos
{
class MockWriteStripeCompletion : public WriteStripeCompletion
{
public:
    using WriteStripeCompletion::WriteStripeCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

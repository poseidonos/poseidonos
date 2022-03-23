#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/wbstripe_manager/read_stripe_completion.h"

namespace pos
{
class MockReadStripeCompletion : public ReadStripeCompletion
{
public:
    using ReadStripeCompletion::ReadStripeCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

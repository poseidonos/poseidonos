#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/wbstripe_manager/read_stripe.h"

namespace pos
{
class MockReadStripe : public ReadStripe
{
public:
    using ReadStripe::ReadStripe;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

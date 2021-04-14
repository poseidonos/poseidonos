#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/backend_io/stripe_map_update_request.h"

namespace pos
{
class MockStripeMapUpdateRequest : public StripeMapUpdateRequest
{
public:
    using StripeMapUpdateRequest::StripeMapUpdateRequest;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

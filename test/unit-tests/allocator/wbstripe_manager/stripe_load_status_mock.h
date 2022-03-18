#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/wbstripe_manager/stripe_load_status.h"

namespace pos
{
class MockStripeLoadStatus : public StripeLoadStatus
{
public:
    using StripeLoadStatus::StripeLoadStatus;
    MOCK_METHOD(void, Reset, (), (override));
    MOCK_METHOD(void, StripeLoadStarted, (), (override));
    MOCK_METHOD(void, StripeLoaded, (), (override));
    MOCK_METHOD(void, StripeLoadFailed, (), (override));
    MOCK_METHOD(bool, IsDone, (), (override));
};

} // namespace pos

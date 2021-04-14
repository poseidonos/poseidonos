#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_locker/stripe_locker.h"

namespace pos
{
class MockStripeLocker : public StripeLocker
{
public:
    using StripeLocker::StripeLocker;
};

} // namespace pos

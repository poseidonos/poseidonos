#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/statistics/stripe_info.h"

namespace pos
{
class MockStripeInfo : public StripeInfo
{
public:
    using StripeInfo::StripeInfo;
};

} // namespace pos

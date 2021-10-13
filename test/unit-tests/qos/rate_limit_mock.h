#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/rate_limit.h"

namespace pos
{
class MockBwIopsRateLimit : public BwIopsRateLimit
{
public:
    using BwIopsRateLimit::BwIopsRateLimit;
};

} // namespace pos

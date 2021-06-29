#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
class MockEventWrapper : public EventWrapper
{
public:
    using EventWrapper::EventWrapper;
};

class MockEventFrameworkApi : public EventFrameworkApi
{
public:
    using EventFrameworkApi::EventFrameworkApi;
};

} // namespace pos

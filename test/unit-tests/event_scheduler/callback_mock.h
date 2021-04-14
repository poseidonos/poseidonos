#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/event_scheduler/callback.h"

namespace pos
{
class MockCallback : public Callback
{
public:
    using Callback::Callback;
    MOCK_METHOD(uint32_t, _GetErrorCount, (), (override));
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

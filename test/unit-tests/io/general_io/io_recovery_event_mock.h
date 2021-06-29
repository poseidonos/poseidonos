#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/io_recovery_event.h"

namespace pos
{
class MockIoRecoveryEvent : public IoRecoveryEvent
{
public:
    using IoRecoveryEvent::IoRecoveryEvent;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

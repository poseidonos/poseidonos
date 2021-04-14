#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/io_recovery_event_factory.h"

namespace pos
{
class MockIoRecoveryEventFactory : public IoRecoveryEventFactory
{
public:
    using IoRecoveryEventFactory::IoRecoveryEventFactory;
    MOCK_METHOD(EventSmartPtr, Create, (UbioSmartPtr ubio), (override));
};

} // namespace pos

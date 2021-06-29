#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/event_scheduler/io_completer.h"

namespace pos
{
class MockIoCompleter : public IoCompleter
{
public:
    using IoCompleter::IoCompleter;
    MOCK_METHOD(void, CompleteOriginUbio, (), (override));
    MOCK_METHOD(void, CompleteUbio, (IOErrorType errorType, bool executeCallback), (override));
    MOCK_METHOD(void, CompleteUbioWithoutRecovery, (IOErrorType errorType, bool executeCallback), (override));
};

} // namespace pos

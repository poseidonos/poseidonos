#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_buffer/callback_sequence_controller.h"

namespace pos
{
class MockCallbackSequenceController : public CallbackSequenceController
{
public:
    using CallbackSequenceController::CallbackSequenceController;
    MOCK_METHOD(void, GetCallbackExecutionApproval, (), (override));
    MOCK_METHOD(void, NotifyCallbackCompleted, (), (override));
    MOCK_METHOD(void, GetCheckpointExecutionApproval, (), (override));
    MOCK_METHOD(void, AllowCallbackExecution, (), (override));
};

} // namespace pos

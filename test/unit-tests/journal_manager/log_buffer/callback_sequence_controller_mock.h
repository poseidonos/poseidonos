#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_buffer/callback_sequence_controller.h"

namespace pos
{
class MockCallbackSequenceController : public CallbackSequenceController
{
public:
    using CallbackSequenceController::CallbackSequenceController;
};

} // namespace pos

#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/replay/replay_progress_reporter.h"

namespace pos
{
class MockReplayProgressReporter : public ReplayProgressReporter
{
public:
    using ReplayProgressReporter::ReplayProgressReporter;
    MOCK_METHOD(void, RegisterTask, (ReplayTaskId taskId, int taskWeight), (override));
};

} // namespace pos

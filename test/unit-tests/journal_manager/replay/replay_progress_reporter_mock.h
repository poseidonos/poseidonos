#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/replay_progress_reporter.h"

namespace pos
{
class MockTaskProgress : public TaskProgress
{
public:
    using TaskProgress::TaskProgress;
};

class MockReplayProgressReporter : public ReplayProgressReporter
{
public:
    using ReplayProgressReporter::ReplayProgressReporter;
};

} // namespace pos

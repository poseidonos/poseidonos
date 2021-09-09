#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/replay/task_progress.h"

namespace pos
{
class MockTaskProgress : public TaskProgress
{
public:
    using TaskProgress::TaskProgress;
};

} // namespace pos

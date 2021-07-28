#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_write/gc_log_write_completed.h"

namespace pos
{
class MockGcStripeFlushedLogWriteCompleted : public GcStripeFlushedLogWriteCompleted
{
public:
    using GcStripeFlushedLogWriteCompleted::GcStripeFlushedLogWriteCompleted;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

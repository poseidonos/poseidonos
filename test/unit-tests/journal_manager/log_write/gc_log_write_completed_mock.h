#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_write/gc_log_write_completed.h"

namespace pos
{
class MockGcLogWriteCompleted : public GcLogWriteCompleted
{
public:
    using GcLogWriteCompleted::GcLogWriteCompleted;
    MOCK_METHOD(void, SetNumLogs, (uint64_t val), (override));
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

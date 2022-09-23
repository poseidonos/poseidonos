#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_write/gc_log_write.h"

namespace pos
{
class MockGcLogWrite : public GcLogWrite
{
public:
    using GcLogWrite::GcLogWrite;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

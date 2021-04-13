#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/gc/gc_flush_submission.h"

namespace pos
{
class MockGcFlushSubmission : public GcFlushSubmission
{
public:
    using GcFlushSubmission::GcFlushSubmission;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

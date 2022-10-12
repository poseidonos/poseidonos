#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/backend_io/flush_submission.h"

namespace pos
{
class MockFlushSubmission : public FlushSubmission
{
public:
    using FlushSubmission::FlushSubmission;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

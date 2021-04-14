#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/backend_io/flush_read_submission.h"

namespace pos
{
class MockFlushReadSubmission : public FlushReadSubmission
{
public:
    using FlushReadSubmission::FlushReadSubmission;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

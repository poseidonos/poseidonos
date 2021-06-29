#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/frontend_io/write_submission.h"

namespace pos
{
class MockWriteSubmission : public WriteSubmission
{
public:
    using WriteSubmission::WriteSubmission;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

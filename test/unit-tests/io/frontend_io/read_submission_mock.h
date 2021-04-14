#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/frontend_io/read_submission.h"

namespace pos
{
class MockReadSubmission : public ReadSubmission
{
public:
    using ReadSubmission::ReadSubmission;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

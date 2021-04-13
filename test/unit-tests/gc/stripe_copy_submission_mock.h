#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/gc/stripe_copy_submission.h"

namespace pos
{
class MockStripeCopySubmission : public StripeCopySubmission
{
public:
    using StripeCopySubmission::StripeCopySubmission;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

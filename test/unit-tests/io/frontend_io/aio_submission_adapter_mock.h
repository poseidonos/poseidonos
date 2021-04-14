#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/frontend_io/aio_submission_adapter.h"

namespace pos
{
class MockAioSubmissionAdapter : public AioSubmissionAdapter
{
public:
    using AioSubmissionAdapter::AioSubmissionAdapter;
    MOCK_METHOD(void, Do, (ibof_io * volIo), (override));
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/submission_adapter.h"

namespace pos
{
class MockSubmissionAdapter : public SubmissionAdapter
{
public:
    using SubmissionAdapter::SubmissionAdapter;
    MOCK_METHOD(int, Do, (UbioSmartPtr bio), (override));
};

class MockIbofIoSubmissionAdapter : public IbofIoSubmissionAdapter
{
public:
    using IbofIoSubmissionAdapter::IbofIoSubmissionAdapter;
    MOCK_METHOD(void, Do, (VolumeIoSmartPtr volIo), (override));
};

} // namespace pos

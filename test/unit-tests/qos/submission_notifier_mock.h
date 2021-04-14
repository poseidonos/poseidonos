#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/submission_notifier.h"

namespace pos
{
class MockSubmissionNotifier : public SubmissionNotifier
{
public:
    using SubmissionNotifier::SubmissionNotifier;
    MOCK_METHOD(void, Do, (uint32_t competionCount), (override));
};

} // namespace pos

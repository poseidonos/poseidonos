#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io_scheduler/io_worker_submission_notifier.h"

namespace pos
{
class MockIOWorkerSubmissionNotifier : public IOWorkerSubmissionNotifier
{
public:
    using IOWorkerSubmissionNotifier::IOWorkerSubmissionNotifier;
    MOCK_METHOD(void, Do, (uint32_t completionCount), (override));
};

} // namespace pos

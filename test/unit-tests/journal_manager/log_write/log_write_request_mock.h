#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_write/log_write_request.h"

namespace pos
{
class MockLogWriteRequest : public LogWriteRequest
{
public:
    using LogWriteRequest::LogWriteRequest;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

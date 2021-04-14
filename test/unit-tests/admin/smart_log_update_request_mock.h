#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/admin/smart_log_update_request.h"

namespace pos
{
class MockSmartLogUpdateRequest : public SmartLogUpdateRequest
{
public:
    using SmartLogUpdateRequest::SmartLogUpdateRequest;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/admin/smart_log_page_handler.h"

namespace pos
{
class MockSmartLogPageHandler : public SmartLogPageHandler
{
public:
    using SmartLogPageHandler::SmartLogPageHandler;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

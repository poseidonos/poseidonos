#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/rebuild/update_data_handler.h"

namespace pos
{
class MockUpdateDataHandler : public UpdateDataHandler
{
public:
    using UpdateDataHandler::UpdateDataHandler;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

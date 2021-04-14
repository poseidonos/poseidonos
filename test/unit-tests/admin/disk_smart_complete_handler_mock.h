#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/admin/disk_smart_complete_handler.h"

namespace pos
{
class MockDiskSmartCompleteHandler : public DiskSmartCompleteHandler
{
public:
    using DiskSmartCompleteHandler::DiskSmartCompleteHandler;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

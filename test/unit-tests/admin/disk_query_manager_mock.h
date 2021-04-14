#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/admin/disk_query_manager.h"

namespace pos
{
class MockGetLogPageContext : public GetLogPageContext
{
public:
    using GetLogPageContext::GetLogPageContext;
};

class MockDiskQueryManager : public DiskQueryManager
{
public:
    using DiskQueryManager::DiskQueryManager;
};

} // namespace pos

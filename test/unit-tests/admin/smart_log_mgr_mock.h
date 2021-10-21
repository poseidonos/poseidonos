#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/admin/smart_log_mgr.h"

namespace pos
{
class MockSmartLogEntry : public SmartLogEntry
{
public:
    using SmartLogEntry::SmartLogEntry;
};

class MockSmartLogMgr : public SmartLogMgr
{
public:
    using SmartLogMgr::SmartLogMgr;
    MOCK_METHOD(bool, GetSmartLogEnabled, (), (override));
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(uint64_t, GetReadBytes, (uint32_t, uint32_t), (override));
    MOCK_METHOD(uint64_t, GetWriteBytes, (uint32_t, uint32_t), (override));
};

} // namespace pos

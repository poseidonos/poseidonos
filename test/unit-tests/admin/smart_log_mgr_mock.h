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

class MockLogPageFlushIoCtx : public LogPageFlushIoCtx
{
public:
    using LogPageFlushIoCtx::LogPageFlushIoCtx;
};

class MockSmartLogMgr : public SmartLogMgr
{
public:
    using SmartLogMgr::SmartLogMgr;
};

} // namespace pos

#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/rebuild/rebuild_logger.h"

namespace pos
{
class MockRebuildLogger : public RebuildLogger
{
public:
    using RebuildLogger::RebuildLogger;
};

} // namespace pos

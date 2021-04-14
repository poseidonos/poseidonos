#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/rebuild/interface/i_rebuild_notification.h"

namespace pos
{
class MockIRebuildNotification : public IRebuildNotification
{
public:
    using IRebuildNotification::IRebuildNotification;
    MOCK_METHOD(int, PrepareRebuild, (string array), (override));
    MOCK_METHOD(void, RebuildDone, (string array), (override));
};

} // namespace pos

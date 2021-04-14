#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/sync_io_completion.h"

namespace pos
{
class MockSyncIoCompletion : public SyncIoCompletion
{
public:
    using SyncIoCompletion::SyncIoCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

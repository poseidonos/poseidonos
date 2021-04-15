#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_write/volume_deleted_log_write_callback.h"

namespace pos
{
class MockVolumeDeletedLogWriteCallback : public VolumeDeletedLogWriteCallback
{
public:
    using VolumeDeletedLogWriteCallback::VolumeDeletedLogWriteCallback;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

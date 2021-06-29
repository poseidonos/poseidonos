#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/uram/uram_restore_completion.h"

namespace pos
{
class MockUramRestoreCompletion : public UramRestoreCompletion
{
public:
    using UramRestoreCompletion::UramRestoreCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

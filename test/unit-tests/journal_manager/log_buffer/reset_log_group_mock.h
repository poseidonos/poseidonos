#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_buffer/reset_log_group.h"

namespace pos
{
class MockResetLogGroup : public ResetLogGroup
{
public:
    using ResetLogGroup::ResetLogGroup;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

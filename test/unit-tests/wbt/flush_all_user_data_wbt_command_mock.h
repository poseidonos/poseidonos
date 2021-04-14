#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/flush_all_user_data_wbt_command.h"

namespace pos
{
class MockFlushAllUserDataWbtCommand : public FlushAllUserDataWbtCommand
{
public:
    using FlushAllUserDataWbtCommand::FlushAllUserDataWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/flush_gcov_data_wbt_command.h"

namespace pos
{
class MockFlushGcovDataWbtCommand : public FlushGcovDataWbtCommand
{
public:
    using FlushGcovDataWbtCommand::FlushGcovDataWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

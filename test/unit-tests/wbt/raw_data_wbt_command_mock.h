#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/raw_data_wbt_command.h"

namespace pos
{
class MockRawDataWbtCommand : public RawDataWbtCommand
{
public:
    using RawDataWbtCommand::RawDataWbtCommand;
    MOCK_METHOD(bool, _VerifySpecificParameters, (Args & argv), (override));
};

} // namespace pos

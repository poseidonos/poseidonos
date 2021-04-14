#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/read_raw_data_wbt_command.h"

namespace pos
{
class MockReadRawDataCommand : public ReadRawDataCommand
{
public:
    using ReadRawDataCommand::ReadRawDataCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
    MOCK_METHOD(bool, _VerifySpecificParameters, (Args & argv), (override));
};

} // namespace pos

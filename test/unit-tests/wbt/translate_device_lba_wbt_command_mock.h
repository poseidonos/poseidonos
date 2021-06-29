#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/translate_device_lba_wbt_command.h"

namespace pos
{
class MockTranslateDeviceLbaWbtCommand : public TranslateDeviceLbaWbtCommand
{
public:
    using TranslateDeviceLbaWbtCommand::TranslateDeviceLbaWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

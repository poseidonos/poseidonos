#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_aligned_file_io_size_wbt_command.h"

namespace pos
{
class MockGetAlignedFileIoSizeWbtCommand : public GetAlignedFileIoSizeWbtCommand
{
public:
    using GetAlignedFileIoSizeWbtCommand::GetAlignedFileIoSizeWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

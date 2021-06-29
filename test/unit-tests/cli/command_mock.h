#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/command.h"

namespace pos
{
class MockPosInfo : public PosInfo
{
public:
    using PosInfo::PosInfo;
};

class MockCommand : public Command
{
public:
    using Command::Command;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mpio_io_info.h"

namespace pos
{
class MockMpioIoInfo : public MpioIoInfo
{
public:
    using MpioIoInfo::MpioIoInfo;
};

class MockMpioTimeInfo : public MpioTimeInfo
{
public:
    using MpioTimeInfo::MpioTimeInfo;
};

} // namespace pos

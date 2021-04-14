#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mpio_handler.h"

namespace pos
{
class MockMpioHandler : public MpioHandler
{
public:
    using MpioHandler::MpioHandler;
};

} // namespace pos

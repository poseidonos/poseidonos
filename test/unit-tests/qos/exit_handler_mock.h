#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/exit_handler.h"

namespace pos
{
class MockExitQosHandler : public ExitQosHandler
{
public:
    using ExitQosHandler::ExitQosHandler;
};

} // namespace pos

#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/signal_handler/signal_handler.h"

namespace pos
{
class MockSignalHandler : public SignalHandler
{
public:
    using SignalHandler::SignalHandler;
};

} // namespace pos

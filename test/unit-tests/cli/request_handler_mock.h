#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/request_handler.h"

namespace pos_cli
{
class MockRequestHandler : public RequestHandler
{
public:
    using RequestHandler::RequestHandler;
};

} // namespace pos_cli

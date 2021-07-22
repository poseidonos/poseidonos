#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/io/general_io/io_submit_handler.h"

namespace pos
{
class MockIOSubmitHandler : public IOSubmitHandler
{
public:
    using IOSubmitHandler::IOSubmitHandler;
};

} // namespace pos

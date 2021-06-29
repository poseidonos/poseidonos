#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/io_controller.h"

namespace pos
{
class MockIOController : public IOController
{
public:
    using IOController::IOController;
};

} // namespace pos

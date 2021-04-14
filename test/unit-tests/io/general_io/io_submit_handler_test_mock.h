#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/io_submit_handler_test.h"

namespace pos
{
class MockIOSubmitHandlerTest : public IOSubmitHandlerTest
{
public:
    using IOSubmitHandlerTest::IOSubmitHandlerTest;
};

} // namespace pos

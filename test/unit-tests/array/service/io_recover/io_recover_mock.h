#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_recover/io_recover.h"

namespace pos
{
class MockIORecover : public IORecover
{
public:
    using IORecover::IORecover;
    MOCK_METHOD(int, GetRecoverMethod, (string array, UbioSmartPtr ubio, RecoverMethod& out), (override));
};

} // namespace pos

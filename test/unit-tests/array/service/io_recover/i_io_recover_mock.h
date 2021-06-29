#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_recover/i_io_recover.h"

namespace pos
{
class MockIIORecover : public IIORecover
{
public:
    using IIORecover::IIORecover;
    MOCK_METHOD(int, GetRecoverMethod, (string array, UbioSmartPtr ubio, RecoverMethod& out), (override));
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_recover/i_recover.h"

namespace pos
{
class MockIRecover : public IRecover
{
public:
    using IRecover::IRecover;
    MOCK_METHOD(int, GetRecoverMethod, (UbioSmartPtr ubio, RecoverMethod& out), (override));
};

} // namespace pos

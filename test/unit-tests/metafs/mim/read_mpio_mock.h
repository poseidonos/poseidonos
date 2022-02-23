#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/read_mpio.h"

namespace pos
{
class MockReadMpio : public ReadMpio
{
public:
    using ReadMpio::ReadMpio;
    MOCK_METHOD(MpioType, GetType, (), (override));
    MOCK_METHOD(void, InitStateHandler, (), (override));
};

} // namespace pos

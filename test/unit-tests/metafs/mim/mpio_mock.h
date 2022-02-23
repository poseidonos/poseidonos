#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mpio.h"

namespace pos
{
class MockMpio : public Mpio
{
public:
    using Mpio::Mpio;
    MOCK_METHOD(MpioType, GetType, (), (override));
    MOCK_METHOD(void, InitStateHandler, (), (override));
};

} // namespace pos

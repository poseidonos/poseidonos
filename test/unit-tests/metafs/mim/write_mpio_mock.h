#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/write_mpio.h"

namespace pos
{
class MockWriteMpio : public WriteMpio
{
public:
    using WriteMpio::WriteMpio;
    MOCK_METHOD(MpioType, GetType, (), (override));
    MOCK_METHOD(void, InitStateHandler, (), (override));
    MOCK_METHOD(void, ExecuteAsyncState, (void* cxt), (override));
};

} // namespace pos

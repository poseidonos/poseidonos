#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/reversemap/reverse_map_io.h"

namespace pos
{
class MockReverseMapIo : public ReverseMapIo
{
public:
    using ReverseMapIo::ReverseMapIo;
    MOCK_METHOD(int, Load, (), (override));
    MOCK_METHOD(int, Flush, (), (override));
    MOCK_METHOD(void, WaitPendingIoDone, (), (override));
};

class MockRevMapPageAsyncIoCtx : public RevMapPageAsyncIoCtx
{
public:
    using RevMapPageAsyncIoCtx::RevMapPageAsyncIoCtx;
    MOCK_METHOD(void, HandleIoComplete, (void* data), (override));
};

} // namespace pos

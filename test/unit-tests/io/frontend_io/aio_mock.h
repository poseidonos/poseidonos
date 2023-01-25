#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/frontend_io/aio.h"

namespace pos
{
class MockIOCtx : public IOCtx
{
public:
    using IOCtx::IOCtx;
};

class MockAioCompletion : public AioCompletion
{
public:
    using AioCompletion::AioCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

class MockAIO : public AIO
{
public:
    using AIO::AIO;
    MOCK_METHOD(void, SubmitAsyncIO, (VolumeIoSmartPtr volIo), (override));
    MOCK_METHOD(VolumeIoSmartPtr, CreatePosReplicatorVolumeIo, (pos_io & posIo, uint64_t lsn), (override));
};

} // namespace pos

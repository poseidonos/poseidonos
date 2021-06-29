#include <gmock/gmock.h>

#include <list>
#include <vector>

#include "src/metafs/mim/metafs_io_scheduler.h"

namespace pos
{
class MockMetaFsIoScheduler : public MetaFsIoScheduler
{
public:
    using MetaFsIoScheduler::MetaFsIoScheduler;
    MOCK_METHOD(void, StartThread, (), (override));
    MOCK_METHOD(bool, EnqueueNewReq, (MetaFsIoRequest* reqMsg), (override));
    MOCK_METHOD(bool, AddArrayInfo, (int arrayId), (override));
    MOCK_METHOD(bool, RemoveArrayInfo, (int arrayId), (override));
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mfs_io_range_overlap_chker.h"

namespace pos
{
class MockMetaFsIoRangeOverlapChker : public MetaFsIoRangeOverlapChker
{
public:
    using MetaFsIoRangeOverlapChker::MetaFsIoRangeOverlapChker;
    MOCK_METHOD(void, Init, (MetaLpnType maxLpn));
    MOCK_METHOD(bool, IsRangeOverlapConflicted, (MetaFsIoRequest* newReq));
    MOCK_METHOD(void, FreeLockContext, (uint64_t startLpn, bool isRead));
    MOCK_METHOD(void, PushReqToRangeLockMap, (MetaFsIoRequest* newReq));
    MOCK_METHOD(BitMap*, GetOutstandingMioMap, ());
    MOCK_METHOD(uint64_t, GetOutstandingMioCount, ());
};

} // namespace pos

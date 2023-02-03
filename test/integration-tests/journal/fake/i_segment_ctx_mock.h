#include "src/allocator/i_segment_ctx.h"

#include <atomic>
#include <gmock/gmock.h>

using ::testing::Mock;

namespace pos
{
class MetaFileIntf;
class AllocatorAddressInfo;
class SegmentInfo;
// struct SegmentInfoMock {
//     std::atomic<uint32_t> validBlkCount;
//     std::atomic<uint32_t> occupiedStripeCount;
// };

class ISegmentCtxMock : public ISegmentCtx, Mock
{
public:
    explicit ISegmentCtxMock(AllocatorAddressInfo* addrInfo, MetaFileIntf* segmentContextFile);
    virtual ~ISegmentCtxMock(void);

    void LoadContext(void);
    int FlushContexts(SegmentInfo* vscSegmentInfos);
    virtual SegmentInfo* GetSegmentInfos(void);

    MOCK_METHOD(void, ValidateBlks, (VirtualBlks blks), (override));
    MOCK_METHOD(bool, InvalidateBlks, (VirtualBlks blks, bool allowVictimSegRelease), (override));
    MOCK_METHOD(bool, UpdateOccupiedStripeCount, (StripeId lsid), (override));
    MOCK_METHOD(void, ValidateBlocksWithGroupId, (VirtualBlks blks, int logGroupId), (override));
    MOCK_METHOD(bool, InvalidateBlocksWithGroupId, (VirtualBlks blks, bool isForced, int logGroupId), (override));
    MOCK_METHOD(bool, UpdateStripeCount, (StripeId lsid, int logGroupId), (override));
    MOCK_METHOD(void, ResetInfos, (SegmentId segId), (override));

private:
    void _ValidateBlks(VirtualBlks blks);
    bool _InvalidateBlks(VirtualBlks blks, bool allowVictimSegRelease);
    bool _UpdateOccupiedStripeCount(StripeId lsid);
    void _SegmentContextReadDone(void);
    
    AllocatorAddressInfo* addrInfo;
    MetaFileIntf* segmentContextFile;
    SegmentInfo* segmentInfos;
    uint64_t fileSize;
    bool segmentContextReadDone;
};

} // namespace pos

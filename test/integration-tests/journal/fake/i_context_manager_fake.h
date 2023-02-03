#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/i_context_manager.h"
#include "src/event_scheduler/event.h"

using ::testing::_;
using ::testing::AtLeast;

namespace pos
{
class ISegmentCtxMock;
class IVersionedSegmentContext;
class AllocatorAddressInfo;
class IContextManagerFake : public IContextManager
{
public:
    explicit IContextManagerFake(ISegmentCtxMock* segmentCtx, AllocatorAddressInfo* addrInfo);
    virtual ~IContextManagerFake(void);

    MOCK_METHOD(int, FlushContexts, (EventSmartPtr callback, bool sync, int logGroupId), (override));
    MOCK_METHOD(uint64_t, GetStoredContextVersion, (int owner), (override));

    virtual void UpdateOccupiedStripeCount(StripeId lsid) {}
    virtual SegmentId AllocateFreeSegment(void) { return 0; }
    virtual SegmentId AllocateGCVictimSegment(void) { return 0; }
    virtual SegmentId AllocateRebuildTargetSegment(void) { return 0; }
    virtual int ReleaseRebuildSegment(SegmentId segmentId) { return 0; }
    virtual int MakeRebuildTarget(void) { return 0; }
    virtual int StopRebuilding(void) { return 0; }
    virtual bool NeedRebuildAgain(void) { return true; }
    virtual uint32_t GetRebuildTargetSegmentCount(void) { return 0; }
    virtual int GetGcThreshold(GcMode mode) { return 0; }
    virtual GcCtx* GetGcCtx(void) { return nullptr; }
    virtual void ResetFlushedInfo(int logGroupId) { return; }
    virtual void SetAllocateDuplicatedFlush(bool flag) { return; }
    
    virtual void PrepareVersionedSegmentCtx(IVersionedSegmentContext* versionedSegCtx_) override;
    IVersionedSegmentContext* GetVersionedSegmentContext(void);
    virtual void SetSegmentContextUpdaterPtr(ISegmentCtx* segmentContextUpdater_) override;
    virtual SegmentCtx* GetSegmentCtx(void) { return nullptr; }
    virtual ISegmentCtx* GetSegmentContextUpdaterPtr(void) override;

private:
    int _FlushContexts(EventSmartPtr callback, bool sync, int logGroupId);
    void _ValidateBlks(VirtualBlks blks);
    void _InvalidateBlks(VirtualBlks blks, bool allowVictimSegRelease);

    AllocatorAddressInfo* addrInfo;
    ISegmentCtxMock* segmentCtx;
    IVersionedSegmentContext* versionedSegCtx;
    bool inInjectedFalut;
};

} // namespace pos

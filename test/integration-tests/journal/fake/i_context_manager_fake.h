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
class ISegmentCtxFake;
class IVersionedSegmentContext;
class AllocatorAddressInfo;
class IContextManagerFake : public IContextManager
{
public:
    explicit IContextManagerFake(ISegmentCtxFake* segmentCtx, AllocatorAddressInfo* addrInfo);
    virtual ~IContextManagerFake(void);

    virtual int FlushContexts(EventSmartPtr callback, bool sync) { return 0; }
    MOCK_METHOD(int, FlushContexts, (EventSmartPtr callback, bool sync, ContextSectionBuffer buffer), (override));

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
    virtual SegmentCtx* GetSegmentCtx(void) { return nullptr; }
    virtual GcCtx* GetGcCtx(void) { return nullptr; }
    
    virtual void SetSegmentContextUpdaterPtr(ISegmentCtx* segmentContextUpdater_) override;
    virtual ISegmentCtx* GetSegmentContextUpdaterPtr(void) override;
    virtual void PrepareVersionedSegmentCtx(IVersionedSegmentContext* versionedSegCtx_) override;
    IVersionedSegmentContext* GetVersionedSegmentContext(void);
    virtual uint64_t GetStoredContextVersion(int owner) override;

private:
    int _FlushContexts(EventSmartPtr callback, bool sync, ContextSectionBuffer buffer);

    ISegmentCtxFake* segmentCtx;
    AllocatorAddressInfo* addrInfo;
    IVersionedSegmentContext* versionedSegCtx;
};

} // namespace pos

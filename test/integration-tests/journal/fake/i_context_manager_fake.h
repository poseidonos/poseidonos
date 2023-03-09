#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/i_context_manager.h"
#include "src/event_scheduler/event.h"
#include "test/integration-tests/journal/fake/segment_ctx_fake.h"

using ::testing::_;
using ::testing::AtLeast;

namespace pos
{
class SegmentCtxFake;
class IVersionedSegmentContext;
class AllocatorAddressInfo;
class IContextManagerFake : public IContextManager
{
public:
    explicit IContextManagerFake(SegmentCtxFake* segmentCtx, AllocatorAddressInfo* addrInfo);
    virtual ~IContextManagerFake(void);

    MOCK_METHOD(int, FlushContexts, (EventSmartPtr callback, bool sync, ContextSectionBuffer buffer), (override));
    virtual uint64_t GetStoredContextVersion(int owner) override;
    virtual SegmentCtx* GetSegmentCtx(void) override;

    virtual int FlushContext(EventSmartPtr callback, ContextSectionBuffer buffer) { return 0; }
    virtual int FlushContexts(EventSmartPtr callback, bool sync) { return 0; }
    virtual SegmentId AllocateFreeSegment(void) { return 0; }
    virtual SegmentId AllocateGCVictimSegment(void) { return 0; }
    virtual SegmentId AllocateRebuildTargetSegment(void) { return 0; }
    virtual int ReleaseRebuildSegment(SegmentId segmentId) { return 0; }
    virtual int StopRebuilding(void) { return 0; }
    virtual bool NeedRebuildAgain(void) { return true; }
    virtual uint32_t GetRebuildTargetSegmentCount(void) { return 0; }
    virtual int GetGcThreshold(GcMode mode) { return 0; }
    virtual GcCtx* GetGcCtx(void) { return nullptr; }

private:
    int _FlushContexts(EventSmartPtr callback, bool sync, ContextSectionBuffer buffer);

    SegmentCtxFake* segmentCtx;
    AllocatorAddressInfo* addrInfo;
    IVersionedSegmentContext* versionedSegCtx;
};

} // namespace pos

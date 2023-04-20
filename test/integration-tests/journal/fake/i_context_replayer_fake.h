#include "src/allocator/i_context_replayer.h"

#include <gmock/gmock.h>
#include <vector>

#include "src/allocator/include/allocator_const.h"

namespace pos
{
class SegmentCtxFake;
class IContextReplayerFake : public IContextReplayer
{
public:
    explicit IContextReplayerFake(SegmentCtxFake* segmentCtx);
    ~IContextReplayerFake(void) = default;

    MOCK_METHOD(void, ResetDirtyContextVersion, (int owner), (override));
    MOCK_METHOD(void, ReplaySsdLsid, (StripeId currentSsdLsid), (override));
    MOCK_METHOD(void, ReplaySegmentAllocation, (StripeId userLsid), (override));
    MOCK_METHOD(void, ReplayStripeAllocation, (StripeId wbLsid, StripeId userLsid), (override));
    MOCK_METHOD(void, ReplayStripeRelease, (StripeId wbLsid), (override));
    MOCK_METHOD(void, ReplayStripeFlushed, (StripeId userLsid), (override));
    MOCK_METHOD(void, ReplayBlockValidated, (VirtualBlks blks), (override));
    MOCK_METHOD(void, ReplayBlockInvalidated, (VirtualBlks blks, bool allowVictimSegRelease), (override));

    MOCK_METHOD(void, SetActiveStripeTail,
        (int index, VirtualBlkAddr tail, StripeId wbLsid), (override));
    MOCK_METHOD(void, ResetActiveStripeTail, (int index), (override));

    virtual void ResetSegmentsStates(void) {}
    virtual std::vector<VirtualBlkAddr> GetAllActiveStripeTail(void);

private:
    void _ReplayStripeFlushed(StripeId userLsid);
    void _ReplayBlockValidated(VirtualBlks blks);
    void _ReplayBlockInvalidated(VirtualBlks blks, bool allowVictimSegRelease);

    SegmentCtxFake* segmentCtx;
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/i_context_manager.h"

namespace pos
{
class IContextManagerMock : public IContextManager
{
public:
    using IContextManager::IContextManager;
    MOCK_METHOD(int, FlushContextsAsync, (EventSmartPtr callback), (override));

    virtual int FlushContextsSync(void) { return 0; }
    virtual void UpdateOccupiedStripeCount(StripeId lsid) {}
    virtual SegmentId AllocateFreeSegment(bool forUser) { return 0; }
    virtual SegmentId AllocateGCVictimSegment(void) { return 0; }
    virtual SegmentId AllocateRebuildTargetSegment(void) { return 0; }
    virtual int ReleaseRebuildSegment(SegmentId segmentId) { return 0; }
    virtual bool NeedRebuildAgain(void) { return true; }
    virtual int GetNumFreeSegment(void) { return 0; }
    virtual CurrentGcMode GetCurrentGcMode(void) { return MODE_NO_GC; }
    virtual int GetGcThreshold(CurrentGcMode mode) { return 0; }
    virtual uint64_t GetStoredContextVersion(int owner) { return 0; }
    IContextManagerMock(void)
    {
        ON_CALL(*this, FlushContextsAsync).WillByDefault(::testing::Invoke(this,
        &IContextManagerMock::_FlushContextsAsync));
    }

private:
    int _FlushContextsAsync(EventSmartPtr callback)
    {
        {
            bool result = callback->Execute();
            if (result == true)
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }
    }
};

} // namespace pos

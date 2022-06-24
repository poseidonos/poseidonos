#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/replay/replay_handler.h"

namespace pos
{
class MockReplayHandler : public ReplayHandler
{
public:
    using ReplayHandler::ReplayHandler;
    MOCK_METHOD(void, Init, (JournalConfiguration* journalConfiguration, IJournalLogBuffer* journalLogBuffer, IVSAMap* vsaMap, IStripeMap* stripeMap, IMapFlush* mapFlush, ISegmentCtx* segmentCtx, IWBStripeAllocator* wbStripeAllocator, IContextManager* contextManager, IContextReplayer* contextReplayer, IArrayInfo* arrayInfo, IVolumeInfoManager* volumeManager), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(int, Start, (), (override));
};

} // namespace pos

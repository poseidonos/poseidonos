#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/journal_manager.h"

namespace pos
{
class MockJournalManager : public JournalManager
{
public:
    using JournalManager::JournalManager;
    MOCK_METHOD(bool, IsEnabled, (), (override));
    MOCK_METHOD(int, Init, (IVSAMap* vsaMap, IStripeMap* stripeMap, IMapFlush* mapFlush, ISegmentCtx* segmentCtx, IWBStripeAllocator* wbStripeAllocator, IContextManager* contextManager, IContextReplayer* contextReplayer, IVolumeManager* volumeManager, MetaFsFileControlApi* metaFsCtrl, EventScheduler* eventScheduler, TelemetryClient* tc), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, Flush, (), (override));
};

} // namespace pos

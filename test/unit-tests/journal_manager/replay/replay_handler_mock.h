#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/replay_handler.h"

namespace pos
{
class MockReplayHandler : public ReplayHandler
{
public:
    using ReplayHandler::ReplayHandler;
    MOCK_METHOD(void, Init, (JournalConfiguration * journalConfiguration, JournalLogBuffer* journalLogBuffer,
                            IVSAMap* vsaMap, IStripeMap* stripeMap, IMapFlush* mapFlush, IBlockAllocator* blockAllocator, IWBStripeAllocator* wbStripeAllocator,
                            IContextManager* contextManager, IContextReplayer* contextReplayer, IArrayInfo* arrayInfo), (override));
    MOCK_METHOD(int, Start, (), (override));
};

} // namespace pos

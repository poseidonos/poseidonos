#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/active_wb_stripe_replayer.h"

namespace pos
{
class MockActiveWBStripeReplayer : public ActiveWBStripeReplayer
{
public:
    using ActiveWBStripeReplayer::ActiveWBStripeReplayer;
    MOCK_METHOD(int, Replay, (), (override));
    MOCK_METHOD(void, Update, (StripeInfo info), (override));
    MOCK_METHOD(void, UpdateRevMaps, (int volId, StripeId vsid, uint64_t offset, BlkAddr startRba), (override));
};

} // namespace pos

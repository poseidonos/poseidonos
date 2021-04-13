#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/gc/victim_stripe.h"

namespace pos
{
class MockBlkInfo : public BlkInfo
{
public:
    using BlkInfo::BlkInfo;
};

class MockVictimStripe : public VictimStripe
{
public:
    using VictimStripe::VictimStripe;
};

} // namespace pos

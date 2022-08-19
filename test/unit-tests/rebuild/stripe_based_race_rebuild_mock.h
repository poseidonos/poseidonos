#pragma once

#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/rebuild/stripe_based_race_rebuild.h"

namespace pos
{
class MockStripeBasedRaceRebuild : public StripeBasedRaceRebuild
{
public:
    using StripeBasedRaceRebuild::StripeBasedRaceRebuild;
    MOCK_METHOD(bool, Rebuild, (), (override));
    MOCK_METHOD(void, UpdateProgress, (uint32_t val), (override));
};

} // namespace pos

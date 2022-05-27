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
    MOCK_METHOD(bool, Init, (), (override));
    MOCK_METHOD(bool, Read, (), (override));
    MOCK_METHOD(bool, Write, (uint32_t targetId, UbioSmartPtr ubio), (override));
    MOCK_METHOD(bool, Complete, (uint32_t targetId, UbioSmartPtr ubio), (override));
    MOCK_METHOD(void, UpdateProgress, (uint32_t val), (override));
    MOCK_METHOD(string, _GetClassName, (), (override));
};

} // namespace pos

#include "src/mapper/map/sequential_page_finder.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MpageSet, CanBeCoalesced_)
{
    MpageSet ms;
    ms.numMpages = 1;
    ms.startMpage = 5;
    ms.CanBeCoalesced(5);
}

TEST(MpageSet, Coalesce_)
{
    MpageSet ms;
    ms.numMpages = 1;
    ms.startMpage = 5;
    ms.Coalesce(4);
}

} // namespace pos

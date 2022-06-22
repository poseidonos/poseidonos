#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/map/sequential_page_finder.h"

namespace pos
{
class MockMpageSet : public MpageSet
{
public:
    using MpageSet::MpageSet;
};

class MockSequentialPageFinder : public SequentialPageFinder
{
public:
    using SequentialPageFinder::SequentialPageFinder;

    MOCK_METHOD(MpageSet, PopNextMpageSet, ());
    MOCK_METHOD(bool, IsRemaining, ());
};

} // namespace pos

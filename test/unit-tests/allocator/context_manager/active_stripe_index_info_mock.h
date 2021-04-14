#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/active_stripe_index_info.h"

namespace pos
{
class MockActiveStripeTailArrIdxInfo : public ActiveStripeTailArrIdxInfo
{
public:
    using ActiveStripeTailArrIdxInfo::ActiveStripeTailArrIdxInfo;
};

} // namespace pos

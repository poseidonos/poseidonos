#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/i_map_flush.h"

namespace pos
{
class MockIMapFlush : public IMapFlush
{
public:
    using IMapFlush::IMapFlush;
    MOCK_METHOD(int, FlushDirtyMpages, (int mapId, EventSmartPtr callback), (override));
    MOCK_METHOD(int, FlushDirtyMpagesGiven, (int mapId, EventSmartPtr callback, MpageList dirtyPages), (override));
    MOCK_METHOD(int, StoreAll, (), (override));
};

} // namespace pos

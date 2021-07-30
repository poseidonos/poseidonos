#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/mapper/mapper.h"

namespace pos
{
class MockMapper : public Mapper
{
public:
    using Mapper::Mapper;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, Flush, (), (override));
    MOCK_METHOD(IVSAMap*, GetIVSAMap, (), (override));
    MOCK_METHOD(IStripeMap*, GetIStripeMap, (), (override));
    MOCK_METHOD(IMapFlush*, GetIMapFlush, (), (override));
    MOCK_METHOD(int, FlushDirtyMpages, (int mapId, EventSmartPtr callback, MpageList dirtyPages), (override));
    MOCK_METHOD(int, StoreAllMaps, (), (override));
};

} // namespace pos

#include "test/integration-tests/journal/fake/mapper_mock.h"

#include <thread>

#include "src/event_scheduler/event.h"

using ::testing::StrictMock;

namespace pos
{
MockMapper::MockMapper(TestInfo* testInfo, IArrayInfo* info, IStateControl* iState)
: Mapper(nullptr, info, nullptr),
  testInfo(testInfo)
{
    flushHandler.resize(testInfo->numMap);
    for (int mapId = 0; mapId < testInfo->numMap; mapId++)
    {
        flushHandler[mapId] = new MapFlushHandlerMock(mapId);
    }

    stripeMapFlushHandler = new MapFlushHandlerMock(STRIPE_MAP_ID);

    ON_CALL(*this, FlushDirtyMpagesGiven).WillByDefault(::testing::Invoke(this,
        &MockMapper::_FlushDirtyMpagesGiven));

    vsaMap = new StrictMock<VSAMapMock>(testInfo);
    stripeMap = new StrictMock<StripeMapMock>(testInfo);
}

MockMapper::~MockMapper(void)
{
    for (int mapId = 0; mapId < testInfo->numMap; mapId++)
    {
        delete flushHandler[mapId];
    }
    delete stripeMapFlushHandler;

    delete vsaMap;
    delete stripeMap;
}

IVSAMap*
MockMapper::GetIVSAMap(void)
{
    return vsaMap;
}

IStripeMap*
MockMapper::GetIStripeMap(void)
{
    return stripeMap;
}

VSAMapMock*
MockMapper::GetVSAMapMock(void)
{
    return vsaMap;
}

StripeMapMock*
MockMapper::GetStripeMapMock(void)
{
    return stripeMap;
}

IMapFlush*
MockMapper::GetIMapFlush(void)
{
    return this;
}

int
MockMapper::StoreAll(void)
{
    return 0;
}

int
MockMapper::_FlushDirtyMpagesGiven(int mapId, EventSmartPtr callback, MpageList dirtyPages)
{
    if (mapId == -1)
    {
        return stripeMapFlushHandler->FlushMapWithPageList(dirtyPages, callback);
    }
    else
    {
        return flushHandler[mapId]->FlushMapWithPageList(dirtyPages, callback);
    }
}
} // namespace pos

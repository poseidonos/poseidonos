#include "test/integration-tests/journal/fake/mapper_mock.h"

#include <thread>

#include "src/event_scheduler/event.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using ::testing::NiceMock;
using ::testing::StrictMock;

namespace pos
{
MockMapper::MockMapper(TestInfo* _testInfo, IArrayInfo* info, IStateControl* iState)
: Mapper(info, nullptr)
{
    testInfo = _testInfo;
    flushHandler.resize(testInfo->numMap);
    for (int mapId = 0; mapId < testInfo->numMap; mapId++)
    {
        flushHandler[mapId] = new MapFlushHandlerMock(mapId);
    }

    stripeMapFlushHandler = new MapFlushHandlerMock(STRIPE_MAP_ID);

    ON_CALL(*this, FlushDirtyMpages).WillByDefault(::testing::Invoke(this,
        &MockMapper::_FlushDirtyMpages));

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
MockMapper::_FlushDirtyMpages(int mapId, EventSmartPtr callback)
{
    if (mapId == -1)
    {
        POS_TRACE_INFO(EID(MAP_FLUSH_STARTED), "Issue Flush StripeMap");
        return stripeMapFlushHandler->FlushTouchedPages(callback);
    }
    else
    {
        POS_TRACE_INFO(EID(MAP_FLUSH_STARTED), "Issue Flush VSAMap, Map ID :{}", mapId);
        return flushHandler[mapId]->FlushTouchedPages(callback);
    }
}
} // namespace pos

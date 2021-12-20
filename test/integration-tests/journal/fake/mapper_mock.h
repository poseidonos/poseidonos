#pragma once

#include <vector>

#include "gmock/gmock.h"
#include "src/mapper/mapper.h"
#include "test/integration-tests/journal/fake/map_flush_handler_mock.h"
#include "test/integration-tests/journal/fake/stripemap_mock.h"
#include "test/integration-tests/journal/fake/vsamap_mock.h"
#include "test/integration-tests/journal/fixture/stripe_test_fixture.h"
#include "test/integration-tests/journal/utils/test_info.h"

using ::testing::StrictMock;

namespace pos
{
class TelemetryPublisher;
class MockMapper : public Mapper
{
public:
    explicit MockMapper(TestInfo* _testInfo, IArrayInfo* info, IStateControl* iState);
    virtual ~MockMapper(void);

    MpageList GetVsaMapDirtyPages(int volId, BlkAddr rba, uint32_t numBlks);
    MpageList GetStripeMapDirtyPages(StripeId vsid);

    MOCK_METHOD(int, FlushDirtyMpages,
        (int mapId, EventSmartPtr callback), (override));

    IVSAMap* GetIVSAMap(void) override;
    IStripeMap* GetIStripeMap(void) override;
    IMapFlush* GetIMapFlush(void) override;

    VSAMapMock* GetVSAMapMock(void);
    StripeMapMock* GetStripeMapMock(void);

    virtual int StoreAll(void) override;

private:
    int _FlushDirtyMpages(int mapId, EventSmartPtr callback);

    TestInfo* testInfo;

    std::vector<MapFlushHandlerMock*> flushHandler;
    MapFlushHandlerMock* stripeMapFlushHandler;

    VSAMapMock* vsaMap;
    StripeMapMock* stripeMap;
};
} // namespace pos

#pragma once

#include "test/integration-tests/journal/fake/array_info_mock.h"
#include "test/integration-tests/journal/fake/mapper_mock.h"
#include "test/integration-tests/journal/fixture/stripe_test_fixture.h"
#include "test/integration-tests/journal/journal_manager_spy.h"
#include "test/integration-tests/journal/utils/rba_generator.h"
#include "test/integration-tests/journal/utils/test_info.h"
#include "test/integration-tests/journal/utils/written_logs.h"

namespace pos
{
class LogWriteTestFixture
{
public:
    LogWriteTestFixture(void) = delete;
    LogWriteTestFixture(MockMapper* _mapper, ArrayInfoMock* _array,
        JournalManagerSpy* _journal, TestInfo* _testInfo);
    virtual ~LogWriteTestFixture(void);

    void Reset(void);
    void UpdateJournal(JournalManagerSpy* _journal);

    bool WriteBlockLog(int volId, BlkAddr rba, VirtualBlks blks);
    bool WriteStripeLog(StripeId vsid, StripeAddr oldAddr, StripeAddr newAddr);
    bool WriteGcStripeLog(int volumeId, StripeId vsid, StripeId wbLsid, StripeId userLsid);
    bool WriteGcStripeLog(int volumeId, StripeTestFixture& stripe);

    void WriteLogsWithSize(uint64_t sizeToFill);

    void GenerateLogsForStripe(StripeTestFixture& stripe, uint32_t startOffset, int numBlks);
    void WriteLogsForStripe(StripeTestFixture& stripe);
    void WriteBlockLogsForStripe(StripeTestFixture& stripe);

    void WriteOverwrittenBlockLogs(StripeTestFixture& stripe,
        BlkAddr rba, uint32_t startOffset, uint32_t numOverwrites);

    void WaitForAllLogWriteDone(void);
    bool AreAllLogWritesDone(void);

    void CompareLogs(void);
    MapList GetDirtyMap(void);

private:
    BlockMapList _GenerateBlocksInStripe(StripeId vsid, uint32_t startOffset, int numBlks);
    void _GenerateGcBlockLogs(GcStripeMapUpdateList& mapUpdates);
    void _AddToDirtyPageList(int mapId);

    std::mutex dirtyListLock;

    WrittenLogs testingLogs;
    MapList dirtyMaps;
    RbaGenerator* rbaGenerator;
    TestInfo* testInfo;

    MockMapper* mapper;
    ArrayInfoMock* array;
    JournalManagerSpy* journal;
};
} // namespace pos

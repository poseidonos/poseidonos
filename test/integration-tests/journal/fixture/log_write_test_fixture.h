#pragma once

#include <unordered_map>
#include <mutex>
#include <utility>
#include <vector>

#include "src/include/address_type.h"
#include "test/integration-tests/journal/fake/allocator_mock.h"
#include "test/integration-tests/journal/fake/array_info_mock.h"
#include "test/integration-tests/journal/fake/mapper_mock.h"
#include "test/integration-tests/journal/fixture/stripe_test_fixture.h"
#include "test/integration-tests/journal/journal_manager_spy.h"
#include "test/integration-tests/journal/utils/rba_generator.h"
#include "test/integration-tests/journal/utils/test_info.h"
#include "test/integration-tests/journal/utils/written_logs.h"

namespace pos
{
class SegmentContextUpdater;
class LogWriteTestFixture
{
public:
    LogWriteTestFixture(void) = delete;
    LogWriteTestFixture(MockMapper* _mapper, AllocatorMock* _allocator, ArrayInfoMock* _array,
        JournalManagerSpy* _journal, TestInfo* _testInfo, SegmentContextUpdater* _segmentContextUpdater);
    virtual ~LogWriteTestFixture(void);

    void Reset(void);
    void UpdateJournal(JournalManagerSpy* _journal);

    bool WriteBlockLog(int volId, BlkAddr rba, VirtualBlks blks);
    bool WriteStripeLog(StripeId vsid, StripeAddr oldAddr, StripeAddr newAddr);
    bool WriteGcStripeLog(int volumeId, StripeId vsid, StripeId wbLsid, StripeId userLsid);
    bool WriteGcStripeLog(int volumeId, StripeTestFixture& stripe);

    void WriteLogsWithSize(uint64_t sizeToFill, SegmentId startSegmentId = 0);

    void GenerateLogsForStripe(StripeTestFixture& stripe, uint32_t startOffset, int numBlks);
    void WriteLogsForStripe(StripeTestFixture& stripe);
    void WriteLogsForStripeWithOverwrittenBlock(StripeTestFixture& stripe, StripeTestFixture& targetStripe);
    void WriteBlockLogsForStripe(StripeTestFixture& stripe);

    void WriteOverwrittenBlockLogs(StripeTestFixture& stripe,
        BlkAddr rba, uint32_t startOffset, uint32_t numOverwrites);

    void WaitForAllLogWriteDone(void);
    bool AreAllLogWritesDone(void);

    void CompareLogs(void);
    MapList GetDirtyMap(void);

    void UpdateMapper(MockMapper* _mapper);

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
    AllocatorMock* allocator;
    JournalManagerSpy* journal;
    SegmentContextUpdater* segmentContextUpdater;
};
} // namespace pos

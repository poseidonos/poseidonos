#pragma once

#include "src/event_scheduler/meta_update_call_back.h"

#include "src/journal_manager/log/log_event.h"

namespace pos
{
class WrittenLogs;
class IContextManagerFake;
class TestInfo;
class SegmentCtx;
class IVersionedSegmentContext;

class TestJournalWriteCompletion : public MetaUpdateCallback
{
public:
    TestJournalWriteCompletion(WrittenLogs* logs, IContextManagerFake* contextManager, TestInfo* testInfo, VirtualBlks blks, LogType eventType);

private:
    bool _DoSpecificJob(void) override;

    WrittenLogs* logs;
    IContextManagerFake* contextManager;
    TestInfo* testInfo;
    VirtualBlks blks;
    LogType eventType;
    ISegmentCtx* segmentCtx;
    IVersionedSegmentContext* versionedSegCtx;
};
} // namespace pos

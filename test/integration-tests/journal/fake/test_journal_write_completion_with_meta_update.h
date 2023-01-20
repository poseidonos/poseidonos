#pragma once

#include "src/event_scheduler/meta_update_call_back.h"

namespace pos
{
class WrittenLogs;
class IVersionedSegmentContext;
class TestInfo;

enum LogEventType
{
    BLOCK_MAP_UPDATE,
    STRIPE_MAP_UPDATE
};
class TestJournalWriteCompletionWithMetaUpdate : public MetaUpdateCallback
{
public:
    TestJournalWriteCompletionWithMetaUpdate(WrittenLogs* logs, IVersionedSegmentContext* versionedContext, TestInfo* testInfo, VirtualBlks blks, LogEventType eventType);

private:
    bool _DoSpecificJob(void) override;

    WrittenLogs* logs;
    IVersionedSegmentContext* versionedContext;
    TestInfo* testInfo;
    VirtualBlks blks;
    LogEventType eventType;
};
} // namespace pos

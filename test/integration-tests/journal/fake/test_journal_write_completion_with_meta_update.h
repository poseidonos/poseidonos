#pragma once

#include "src/event_scheduler/meta_update_call_back.h"

namespace pos
{
class WrittenLogs;
class IContextManagerFake;
class TestInfo;
class SegmentCtx;
class IVersionedSegmentContext;

enum LogEventType
{
    BLOCK_MAP_UPDATE,
    STRIPE_MAP_UPDATE
};
class TestJournalWriteCompletionWithMetaUpdate : public MetaUpdateCallback
{
public:
    TestJournalWriteCompletionWithMetaUpdate(WrittenLogs* logs, IContextManagerFake* contextManager, TestInfo* testInfo, VirtualBlks blks, LogEventType eventType);

private:
    bool _DoSpecificJob(void) override;

    WrittenLogs* logs;
    IContextManagerFake* contextManager;
    TestInfo* testInfo;
    VirtualBlks blks;
    LogEventType eventType;
    ISegmentCtx* segmentCtx;
    IVersionedSegmentContext* versionedSegCtx;
};
} // namespace pos

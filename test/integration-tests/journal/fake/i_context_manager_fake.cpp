#include "i_context_manager_fake.h"

#include <thread>

#include "src/allocator/address/allocator_address_info.h"
#include "test/integration-tests/journal/fake/i_segment_ctx_fake.h"

namespace pos
{
IContextManagerFake::IContextManagerFake(ISegmentCtxFake* segmentCtx, AllocatorAddressInfo* addrInfo)
: segmentCtx(segmentCtx),
  addrInfo(addrInfo)
{
    ON_CALL(*this, FlushContexts).WillByDefault(::testing::Invoke(this, &IContextManagerFake::_FlushContexts));
    EXPECT_CALL(*this, FlushContexts(_, true, _)).Times(AtLeast(0));
}

IContextManagerFake::~IContextManagerFake(void)
{
}

void
IContextManagerFake::SetSegmentContextUpdaterPtr(ISegmentCtx* segmentContextUpdater_)
{
    segmentCtx = (ISegmentCtxFake*)segmentContextUpdater_;
}

ISegmentCtx*
IContextManagerFake::GetSegmentContextUpdaterPtr(void)
{
    return segmentCtx;
}

void
IContextManagerFake::PrepareVersionedSegmentCtx(IVersionedSegmentContext* versionedSegCtx_)
{
    versionedSegCtx = versionedSegCtx_;
}

IVersionedSegmentContext*
IContextManagerFake::GetVersionedSegmentContext(void)
{
    return versionedSegCtx;
}

uint64_t
IContextManagerFake::GetStoredContextVersion(int owner)
{
    uint64_t contextVersion = segmentCtx->GetStoredVersion();
    return contextVersion;
}

int
IContextManagerFake::_FlushContexts(EventSmartPtr callback, bool sync, int logGroupId)
{
    SegmentInfoData* vscSegInfoData = (true == sync) ? nullptr : versionedSegCtx->GetUpdatedInfoDataToFlush(logGroupId);
    segmentCtx->FlushContexts(vscSegInfoData);
    if (callback != nullptr)
    {
        std::thread eventExecution(&Event::Execute, callback);
        eventExecution.detach();
    }
    return 0;
}
} // namespace pos

#include "i_context_manager_fake.h"

#include "src/allocator/address/allocator_address_info.h"
#include "test/integration-tests/journal/fake/i_segment_ctx_mock.h"

namespace pos
{
IContextManagerFake::IContextManagerFake(ISegmentCtxMock* segmentCtx, AllocatorAddressInfo* addrInfo)
: segmentCtx(segmentCtx),
  addrInfo(addrInfo),
  inInjectedFalut(false)
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
    segmentCtx = (ISegmentCtxMock*)segmentContextUpdater_;
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

int
IContextManagerFake::_FlushContexts(EventSmartPtr callback, bool sync, int logGroupId)
{
    SegmentInfo* vscSegInfo = (true == sync) ? nullptr : versionedSegCtx->GetUpdatedInfoToFlush(logGroupId);

    segmentCtx->FlushContexts(vscSegInfo);
    std::thread eventExecution(&Event::Execute, callback);
    eventExecution.detach();
    return 0;
}
} // namespace pos

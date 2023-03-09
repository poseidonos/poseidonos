#include "i_context_manager_fake.h"

#include <thread>

#include "src/allocator/address/allocator_address_info.h"
#include "src/metadata/segment_context_updater.h"
#include "test/integration-tests/journal/fake/segment_ctx_fake.h"

namespace pos
{
IContextManagerFake::IContextManagerFake(SegmentCtxFake* segmentCtx, AllocatorAddressInfo* addrInfo)
: segmentCtx(segmentCtx),
  addrInfo(addrInfo)
{
    ON_CALL(*this, FlushContexts).WillByDefault(::testing::Invoke(this, &IContextManagerFake::_FlushContexts));
}

IContextManagerFake::~IContextManagerFake(void)
{
}

uint64_t
IContextManagerFake::GetStoredContextVersion(int owner)
{
    uint64_t contextVersion = segmentCtx->GetStoredVersion();
    return contextVersion;
}

SegmentCtx*
IContextManagerFake::GetSegmentCtx(void)
{
    return dynamic_cast<SegmentCtx*>(segmentCtx);
}

int
IContextManagerFake::_FlushContexts(EventSmartPtr callback, bool sync, ContextSectionBuffer buffer)
{
    segmentCtx->FlushContexts(reinterpret_cast<SegmentInfoData*>(buffer.buffer));
    if (callback != nullptr)
    {
        std::thread eventExecution(&Event::Execute, callback);
        eventExecution.detach();
    }
    return 0;
}
} // namespace pos

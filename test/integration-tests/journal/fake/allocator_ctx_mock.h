#pragma once

#include "gmock/gmock.h"
#include "src/allocator/i_allocator_ctx.h"

namespace pos
{
class AllocatorCtxMock : public IAllocatorCtx
{
public:
    AllocatorCtxMock(void);
    virtual ~AllocatorCtxMock(void);

    MOCK_METHOD(int, FlushAllocatorCtxs,
        (EventSmartPtr callback), (override));

    virtual int StoreAllocatorCtxs(void) override { return 0; }
    virtual uint64_t GetAllocatorCtxsStoredVersion(void) override { return 0; }
    virtual void ResetAllocatorCtxsDirtyVersion(void) override {}

private:
    int _FlushAllocatorCtxs(EventSmartPtr callback);
};

} // namespace pos

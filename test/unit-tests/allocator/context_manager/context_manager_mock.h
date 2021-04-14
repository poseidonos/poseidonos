#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/context_manager.h"

namespace pos
{
class MockCtxHeader : public CtxHeader
{
public:
    using CtxHeader::CtxHeader;
};

class MockContextManager : public ContextManager
{
public:
    using ContextManager::ContextManager;
    MOCK_METHOD(int, FlushAllocatorCtxs, (EventSmartPtr callback), (override));
    MOCK_METHOD(int, StoreAllocatorCtxs, (), (override));
    MOCK_METHOD(uint64_t, GetAllocatorCtxsStoredVersion, (), (override));
    MOCK_METHOD(void, ResetAllocatorCtxsDirtyVersion, (), (override));
    MOCK_METHOD(void, ReplayStripeAllocation, (StripeId vsid, StripeId wbLsid), (override));
    MOCK_METHOD(void, ReplayStripeFlushed, (StripeId wbLsid), (override));
    MOCK_METHOD(std::vector<VirtualBlkAddr>, GetAllActiveStripeTail, (), (override));
    MOCK_METHOD(void, ResetActiveStripeTail, (int index), (override));
    MOCK_METHOD(std::mutex&, GetCtxLock, (), (override));
};

} // namespace pos

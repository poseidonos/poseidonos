#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mfs_async_runnable_template.h"

namespace pos
{
template<typename CallbackCxtT, typename AsyncStateExecutionEntry, typename AsyncStateT>
class MockMetaAsyncRunnable : public MetaAsyncRunnable<CallbackCxtT, AsyncStateExecutionEntry, AsyncStateT>
{
public:
    using MetaAsyncRunnable<CallbackCxtT, AsyncStateExecutionEntry, AsyncStateT>::MetaAsyncRunnable;
    MOCK_METHOD(void, InitStateHandler, (), (override));
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, RegisterStateHandler, (AsyncStateT state, AsyncStateExecutionEntry* entry), (override));
    MOCK_METHOD(void, ExecuteAsyncState, (void* cxt = nullptr), (override));
    MOCK_METHOD(void, InvokeClientCallback, (), (override));
    MOCK_METHOD(void, SetAsyncCbCxt, (CallbackCxtT* cxt, bool deleteRequired = true), (override));
};

} // namespace pos

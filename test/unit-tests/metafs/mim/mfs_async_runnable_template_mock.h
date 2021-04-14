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
    using MetaAsyncRunnable::MetaAsyncRunnable;
    MOCK_METHOD(void, InitStateHandler, (), (override));
};

} // namespace pos

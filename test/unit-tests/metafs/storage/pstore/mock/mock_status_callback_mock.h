#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/storage/pstore/mock/mock_status_callback.h"

namespace pos
{
class MockMockStatusCallback : public MockStatusCallback
{
public:
    using MockStatusCallback::MockStatusCallback;
};

} // namespace pos

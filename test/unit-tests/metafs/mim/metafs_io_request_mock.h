#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/metafs_io_request.h"

namespace pos
{
class MockMetaFsIoRequest : public MetaFsIoRequest
{
public:
    using MetaFsIoRequest::MetaFsIoRequest;
    MOCK_METHOD(bool, IsValid, (), (override));
    MOCK_METHOD(bool, IsSyncIO, (), (override));
    MOCK_METHOD(bool, IsIoCompleted, (), (override));
    MOCK_METHOD(bool, GetError, (), (override));
    MOCK_METHOD(void, SuspendUntilIoCompletion, (), (override));
    MOCK_METHOD(void, NotifyIoCompletionToClient, (), (override));
    MOCK_METHOD(void, SetError, (bool err), (override));
    MOCK_METHOD(void, SetRetryFlag, (), (override));
    MOCK_METHOD(bool, GetRetryFlag, (), (override));
    MOCK_METHOD(MetaLpnType, GetStartLpn, (), (const));
    MOCK_METHOD(size_t, GetRequestLpnCount, (), (const));
    MOCK_METHOD(MetaFileType, GetFileType, (), (const));
};

} // namespace pos

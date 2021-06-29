#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/storage/pstore/mss_disk_handler.h"

namespace pos
{
class MockMssCompleteHandler : public MssCompleteHandler
{
public:
    using MssCompleteHandler::MssCompleteHandler;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

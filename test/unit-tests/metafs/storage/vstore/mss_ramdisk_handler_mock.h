#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/storage/vstore/mss_ramdisk_handler.h"

namespace pos
{
class MockMssCompleteHandler : public MssCompleteHandler
{
public:
    using MssCompleteHandler::MssCompleteHandler;
};

} // namespace pos

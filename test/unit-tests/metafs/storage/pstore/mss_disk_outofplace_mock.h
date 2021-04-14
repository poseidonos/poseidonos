#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/storage/pstore/mss_disk_outofplace.h"

namespace pos
{
class MockMssDiskOutOfPlace : public MssDiskOutOfPlace
{
public:
    using MssDiskOutOfPlace::MssDiskOutOfPlace;
};

} // namespace pos

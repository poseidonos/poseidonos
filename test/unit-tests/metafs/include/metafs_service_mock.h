#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/include/metafs_service.h"

namespace pos
{
class MockMetaFsService : public MetaFsService
{
public:
    using MetaFsService::MetaFsService;
};

}   // namespace pos

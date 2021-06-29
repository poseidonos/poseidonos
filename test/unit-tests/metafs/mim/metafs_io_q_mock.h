#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/metafs_io_q.h"

namespace pos
{
template<typename T>
class MockMetaFsIoQ : public MetaFsIoQ<T>
{
public:
    using MetaFsIoQ::MetaFsIoQ;
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/scalable_meta_io_worker.h"

namespace pos
{
class MockScalableMetaIoWorker : public ScalableMetaIoWorker
{
public:
    using ScalableMetaIoWorker::ScalableMetaIoWorker;
    MOCK_METHOD(void, StartThread, (), (override));
};

} // namespace pos

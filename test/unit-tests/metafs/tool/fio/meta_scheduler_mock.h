#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/tool/fio/meta_scheduler.h"

namespace pos
{
class MockMetaIoHandler : public MetaIoHandler
{
public:
    using MetaIoHandler::MetaIoHandler;
};

class MockMetaIOScheduler : public MetaIOScheduler
{
public:
    using MetaIOScheduler::MetaIOScheduler;
};

class MockMetaFioAIOCxt : public MetaFioAIOCxt
{
public:
    using MetaFioAIOCxt::MetaFioAIOCxt;
};

} // namespace pos

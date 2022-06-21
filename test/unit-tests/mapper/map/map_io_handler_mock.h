#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/map/map_io_handler.h"

namespace pos
{
class MockMapFlushIoContext : public MapFlushIoContext
{
public:
    using MapFlushIoContext::MapFlushIoContext;
};

class MockMapIoHandler : public MapIoHandler
{
public:
    using MapIoHandler::MapIoHandler;

    MOCK_METHOD(int, CreateFlushRequestFor, (MpageNum start, int numPages, MetaIoCbPtr callback));
    MOCK_METHOD(void, CreateFlushEvents, (std::unique_ptr<SequentialPageFinder> sequentialPages));
};

} // namespace pos

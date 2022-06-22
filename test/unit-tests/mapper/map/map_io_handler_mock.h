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

    MOCK_METHOD(int, CreateFlushRequestFor, (const MpageSet& mpageSet), (override));
    MOCK_METHOD(void, CreateFlushEvents, (std::unique_ptr<SequentialPageFinder> sequentialPages), (override));
};

} // namespace pos

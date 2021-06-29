#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/map/map_flush_handler.h"

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
};

} // namespace pos

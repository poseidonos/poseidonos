
#include "src/collection/CollectionManager.cpp"
#include "src/collection/CollectionManager.h"
#include "src/collection/CollectionObserver.cpp"
#include "src/collection/CollectionObserver.h"
#include "src/collection/Collector.cpp"
#include "src/collection/writer/LatencyWriter.cpp"
#include "src/collection/writer/LatencyWriter.h"
#include "src/collection/writer/PerformanceWriter.cpp"
#include "src/collection/writer/PerformanceWriter.h"
#include "src/collection/writer/QueueWriter.cpp"
#include "src/collection/writer/QueueWriter.h"
#include "src/collection/writer/Writer.cpp"
#include "src/collection/writer/Writer.h"
#include "src/stream/Stream.cpp"
#include "src/stream/Stream.h"

class MockCollectionObserver : public collection::Observer
{
public:
    virtual ~MockCollectionObserver()
    {
    }
    virtual void
    Update(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2, int pid, int cmd_type, int cmd_order)
    {
        return;
    }
    virtual void
    Handle()
    {
        return;
    }

private:
};

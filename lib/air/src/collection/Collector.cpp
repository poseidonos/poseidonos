
#include "src/collection/Collector.h"

collection::Collector::~Collector(void)
{
    if (nullptr != writer)
    {
        delete writer;
        writer = nullptr;
    }
}

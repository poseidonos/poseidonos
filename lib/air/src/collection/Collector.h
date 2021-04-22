
#ifndef AIR_COLLECTOR_H
#define AIR_COLLECTOR_H

#include "src/collection/writer/Writer.h"

namespace collection
{
class Collector
{
public:
    explicit Collector(Writer* new_writer)
    : writer(new_writer)
    {
    }
    virtual ~Collector(void);
    int
    SetSamplingRate(uint32_t rate)
    {
        return writer->SetSamplingRate(rate);
    }
    inline void
    LogData(lib::Data* data, uint64_t value1, uint64_t value2)
    {
        if (nullptr != data)
        {
            writer->LogData(data, value1, value2);
        }
    }
    void
    InformInit(lib::AccData* data)
    {
        if (nullptr != data)
        {
            writer->InformInit(data);
        }
    }

protected:
    Writer* writer{nullptr};
};

class PerformanceCollector : public Collector
{
public:
    explicit PerformanceCollector(Writer* new_writer)
    : Collector(new_writer)
    {
    }
    virtual ~PerformanceCollector(void)
    {
    }
};

class LatencyCollector : public Collector
{
public:
    explicit LatencyCollector(Writer* new_writer)
    : Collector(new_writer)
    {
    }
    virtual ~LatencyCollector(void)
    {
    }
};

class QueueCollector : public Collector
{
public:
    explicit QueueCollector(Writer* new_writer)
    : Collector(new_writer)
    {
    }
    virtual ~QueueCollector(void)
    {
    }
};

class UtilizationCollector : public Collector
{
public:
    explicit UtilizationCollector(Writer* new_writer)
    : Collector(new_writer)
    {
    }
    virtual ~UtilizationCollector(void)
    {
    }
};

class CountCollector : public Collector
{
public:
    explicit CountCollector(Writer* new_writer)
    : Collector(new_writer)
    {
    }
    virtual ~CountCollector(void)
    {
    }
};

} // namespace collection

#endif // AIR_COLLECTOR_H

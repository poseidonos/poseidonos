
#include "src/collection/Writer.h"

#include <string.h>

void
collection::Writer::InformInit(lib::AccData* data)
{
    if (nullptr == data)
    {
        return;
    }
    data->need_erase = 1;
}

void
collection::QueueWriter::_UpdateRand(void)
{
    mersenne.seed(std::rand());
}

int
collection::QueueWriter::SetSamplingRate(uint32_t rate)
{
    if (rate >= MIN_RATIO && rate <= MAX_RATIO)
    {
        sampling_rate = rate;
        _UpdateRand();

        return 0;
    }
    else
    {
        return -2; // boundray error
    }
}

int
collection::PerformanceWriter::SetSamplingRate(uint32_t rate)
{
    return 0;
}

int
collection::LatencyWriter::SetSamplingRate(uint32_t rate)
{
    return 0;
}

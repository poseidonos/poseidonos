
#include "src/meta/GlobalMeta.h"

class MockGlobalMeta : public meta::GlobalMeta
{
public:
    virtual ~MockGlobalMeta()
    {
    }
    virtual inline bool
    Enable() const
    {
        return true;
    }
    virtual inline void
    SetEnable(bool new_enable)
    {
        return;
    }
    virtual inline uint32_t
    StreamingInterval() const
    {
        return interval;
    }
    virtual inline void
    SetStreamingInterval(uint32_t new_streaming_interval)
    {
        interval = new_streaming_interval;
        return;
    }

private:
    uint32_t interval{1};
};

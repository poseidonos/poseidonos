
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
    virtual inline uint32_t
    StreamingInterval() const
    {
        return interval;
    }
    virtual inline bool
    StreamingUpdate() const
    {
        return true;
    }
    virtual inline void
    UpdateStreamingInterval()
    {
        return;
    }

private:
    uint32_t interval{3};
};


#include "src/meta/GlobalMeta.h"

class MockGlobalMetaGetter : public meta::GlobalMetaGetter
{
public:
    virtual ~MockGlobalMetaGetter()
    {
    }
    virtual inline bool
    AirPlay() const
    {
        return true;
    }
    virtual inline bool
    Enable() const
    {
        return true;
    }
    virtual inline uint32_t
    StreamingInterval() const
    {
        return 1;
    }
    virtual inline uint32_t
    NextStreamingInterval() const
    {
        return 1;
    }
    virtual inline uint32_t
    NodeIndexSize() const
    {
        return 32;
    }

private:
};

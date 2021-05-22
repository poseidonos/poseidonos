
#include "src/meta/GlobalMeta.h"

class MockGlobalMetaGetter : public meta::GlobalMetaGetter
{
public:
    virtual ~MockGlobalMetaGetter()
    {
    }

    inline uint32_t
    NodeIndexSize() const
    {
        return 32;
    }

private:
};

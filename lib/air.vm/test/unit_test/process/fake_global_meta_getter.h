
#ifndef FAKE_GLOBAL_META_GETTER_H
#define FAKE_GLOBAL_META_GETTER_H

#include "src/meta/GlobalMeta.h"

class FakeGlobalMetaGetter : public meta::GlobalMetaGetter
{
public:
    FakeGlobalMetaGetter()
    {
        enable = true;
    }
    virtual ~FakeGlobalMetaGetter()
    {
    }

    bool
    Enable() const
    {
        return enable;
    }

    void
    SetEnable(bool new_enable)
    {
        enable = new_enable;
    }
    uint32_t
    StreamingInterval() const
    {
        return 1;
    }
    uint32_t
    NodeIndexSize() const
    {
        return 32;
    }

private:
    bool enable;
};

#endif // #define FAKE_GLOBAL_META_GETTER_H

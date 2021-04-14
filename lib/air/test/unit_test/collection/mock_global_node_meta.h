
#include "src/meta/GlobalMeta.h"

class MockGlobalMetaGetter : public meta::GlobalMetaGetter
{
public:
    virtual ~MockGlobalMetaGetter() {}
    virtual inline bool Enable() const {
        return true;
    }
    virtual inline uint32_t AidSize() const {
        return 32;
    }
    virtual inline uint32_t StreamingInterval() const {
        return 1;
    }
    virtual inline uint32_t NextStreamingInterval() const {
        return 1;
    }
private:
};

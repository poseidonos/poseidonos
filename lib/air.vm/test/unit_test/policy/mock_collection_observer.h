
#include "src/collection/CollectionObserver.h"
#include "src/collection/CollectionObserver.cpp"
#include "src/collection/CollectionManager.h"
#include "src/collection/CollectionManager.cpp"
#include "src/collection/Collector.cpp"
#include "src/collection/Writer.cpp"
#include "src/stream/Stream.h"
#include "src/stream/Stream.cpp"

class MockCollectionObserver : public collection::Observer
{
public:
    virtual ~MockCollectionObserver() {}
    virtual void Update(uint32_t type1, uint32_t type2, uint32_t value1,
                        uint32_t value2, int pid, int cmd_type, int cmd_order) {
        return;
    }
    virtual void Handle() {
        return;
    }
private:
};

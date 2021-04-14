
#ifndef AIR_COLLECTION_COR_HANDLER_H
#define AIR_COLLECTION_COR_HANDLER_H

#include "src/collection/CollectionObserver.h"
#include "src/lib/Design.h"

namespace collection
{
class CollectionCoRHandler : public lib_design::AbstractCoRHandler
{
public:
    explicit CollectionCoRHandler(collection::Observer* new_observer)
    : observer(new_observer)
    {
    }
    virtual ~CollectionCoRHandler(void)
    {
    }
    virtual void
    HandleRequest(int option = 0)
    {
        observer->Handle();
    }

private:
    collection::Observer* observer{nullptr};
};

} // namespace collection

#endif // AIR_COLLECTION_COR_HANDLER_H

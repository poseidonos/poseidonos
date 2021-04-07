
#ifndef AIR_COLLECTION_OBSERVER_H
#define AIR_COLLECTION_OBSERVER_H

#include "src/collection/CollectionManager.h"
#include "src/lib/Design.h"

namespace collection
{
class Observer : public lib_design::Observer
{
public:
    Observer(void)
    {
    }
    explicit Observer(CollectionManager* new_collection_manager)
    : collection_manager(new_collection_manager)
    {
    }
    virtual ~Observer(void)
    {
    }
    virtual void Update(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2, int pid, int cmd_type, int cmd_order);
    virtual void Handle(void);

private:
    CollectionManager* collection_manager{nullptr};
};

} // namespace collection

#endif // AIR_COLLECTION_OBSERVER_H

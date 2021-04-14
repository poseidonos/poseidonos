#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator_service/allocator_interface_container.h"

namespace pos
{
template<typename T>
class MockAllocatorInterfaceContainer : public AllocatorInterfaceContainer<T>
{
public:
    using AllocatorInterfaceContainer::AllocatorInterfaceContainer;
};

} // namespace pos

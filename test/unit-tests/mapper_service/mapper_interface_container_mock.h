#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper_service/mapper_interface_container.h"

namespace pos
{
template<typename T>
class MockMapperInterfaceContainer : public MapperInterfaceContainer<T>
{
public:
    using MapperInterfaceContainer::MapperInterfaceContainer;
};

} // namespace pos

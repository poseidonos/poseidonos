#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/internal_manager_factory.h"

namespace pos
{
class MockInternalManagerFactory : public InternalManagerFactory
{
public:
    using InternalManagerFactory::InternalManagerFactory;
};

} // namespace pos

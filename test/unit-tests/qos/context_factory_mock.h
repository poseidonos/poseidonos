#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/context_factory.h"

namespace pos
{
class MockContextFactory : public ContextFactory
{
public:
    using ContextFactory::ContextFactory;
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/master_context/default_configuration.h"

namespace pos
{
class MockDefaultConfiguration : public DefaultConfiguration
{
public:
    using DefaultConfiguration::DefaultConfiguration;
};

} // namespace pos

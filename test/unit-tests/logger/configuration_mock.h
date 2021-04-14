#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/logger/configuration.h"

namespace pos_logger
{
class MockConfiguration : public Configuration
{
public:
    using Configuration::Configuration;
};

} // namespace pos_logger

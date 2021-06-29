#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/frontend_io/flush_configuration.h"

namespace pos
{
class MockFlushConfiguration : public FlushConfiguration
{
public:
    using FlushConfiguration::FlushConfiguration;
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/logger/filter.h"

namespace pos_logger
{
class MockFilter : public Filter
{
public:
    using Filter::Filter;
};

} // namespace pos_logger

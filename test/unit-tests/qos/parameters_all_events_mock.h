#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/parameters_all_events.h"

namespace pos
{
class MockAllEventParameter : public AllEventParameter
{
public:
    using AllEventParameter::AllEventParameter;
};

} // namespace pos

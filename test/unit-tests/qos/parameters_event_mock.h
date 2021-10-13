#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/parameters_event.h"

namespace pos
{
class MockEventParameter : public EventParameter
{
public:
    using EventParameter::EventParameter;
};

} // namespace pos

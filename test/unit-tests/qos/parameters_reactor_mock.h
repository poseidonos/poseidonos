#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/parameters_reactor.h"

namespace pos
{
class MockReactorParameter : public ReactorParameter
{
public:
    using ReactorParameter::ReactorParameter;
};

} // namespace pos

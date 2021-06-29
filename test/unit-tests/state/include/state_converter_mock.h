#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/state/include/state_converter.h"

namespace pos
{
class MockStateConverter : public StateConverter
{
public:
    using StateConverter::StateConverter;
};

} // namespace pos

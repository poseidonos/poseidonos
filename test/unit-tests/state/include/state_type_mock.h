#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/state/include/state_type.h"

namespace pos
{
class MockStateType : public StateType
{
public:
    using StateType::StateType;
};

} // namespace pos

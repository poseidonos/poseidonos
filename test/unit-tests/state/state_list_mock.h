#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/state/state_list.h"

namespace pos
{
class MockStateList : public StateList
{
public:
    using StateList::StateList;
};

} // namespace pos

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
    MOCK_METHOD(void, Add, (StateContext * ctx), (override));
    MOCK_METHOD(void, Remove, (StateContext * ctx), (override));
    MOCK_METHOD(bool, Exists, (SituationEnum situ), (override));
};

} // namespace pos

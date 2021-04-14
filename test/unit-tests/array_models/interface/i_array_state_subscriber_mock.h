#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_models/interface/i_array_state_subscriber.h"

namespace pos
{
class MockIArrayStateSubscriber : public IArrayStateSubscriber
{
public:
    using IArrayStateSubscriber::IArrayStateSubscriber;
    MOCK_METHOD(void, StateChanged, (ArrayStateType & state), (override));
};

} // namespace pos

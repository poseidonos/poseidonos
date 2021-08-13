#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array_components/array_mount_sequence.h"

namespace pos
{
class MockArrayMountSequence : public ArrayMountSequence
{
public:
    using ArrayMountSequence::ArrayMountSequence;
    MOCK_METHOD(int, Mount, (), (override));
    MOCK_METHOD(int, Unmount, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, StateChanged, (StateContext* prev, StateContext* next), (override));
};

} // namespace pos

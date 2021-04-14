#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_components/array_mount_sequence.h"

namespace pos
{
class MockArrayMountSequence : public ArrayMountSequence
{
public:
    using ArrayMountSequence::ArrayMountSequence;
    MOCK_METHOD(void, StateChanged, (StateContext * prev, StateContext* next), (override));
};

} // namespace pos

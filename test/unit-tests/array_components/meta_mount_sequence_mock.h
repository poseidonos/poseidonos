#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_components/meta_mount_sequence.h"

namespace pos
{
class MockMetaMountSequence : public MetaMountSequence
{
public:
    using MetaMountSequence::MetaMountSequence;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
};

} // namespace pos

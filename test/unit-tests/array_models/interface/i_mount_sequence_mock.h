#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_models/interface/i_mount_sequence.h"

namespace pos
{
class MockIMountSequence : public IMountSequence
{
public:
    using IMountSequence::IMountSequence;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, Flush, (), (override));
};

} // namespace pos

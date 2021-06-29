#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/array_unlocking.h"

namespace pos
{
class MockArrayUnlocking : public ArrayUnlocking
{
public:
    using ArrayUnlocking::ArrayUnlocking;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

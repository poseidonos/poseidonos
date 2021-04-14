#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/rebuild/rebuilder.h"

namespace pos
{
class MockRebuilder : public Rebuilder
{
public:
    using Rebuilder::Rebuilder;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

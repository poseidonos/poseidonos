#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/gc/stripe_copier.h"

namespace pos
{
class MockStripeCopier : public StripeCopier
{
public:
    using StripeCopier::StripeCopier;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

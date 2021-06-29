#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/state/include/situation_type.h"

namespace pos
{
class MockSituationType : public SituationType
{
public:
    using SituationType::SituationType;
};

} // namespace pos

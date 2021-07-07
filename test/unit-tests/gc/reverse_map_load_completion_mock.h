#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/gc/reverse_map_load_completion.h"

namespace pos
{
class MockReverseMapLoadCompletion : public ReverseMapLoadCompletion
{
public:
    using ReverseMapLoadCompletion::ReverseMapLoadCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

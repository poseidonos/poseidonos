#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/vsa_range_maker.h"

namespace pos
{
class MockVsaRangeMaker : public VsaRangeMaker
{
public:
    using VsaRangeMaker::VsaRangeMaker;
    MOCK_METHOD(uint32_t, GetCount, (), (override));
    MOCK_METHOD(VirtualBlks&, GetVsaRange, (uint32_t index), (override));
};

} // namespace pos

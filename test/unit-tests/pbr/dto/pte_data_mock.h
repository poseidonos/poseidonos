#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/dto/pte_data.h"

namespace pbr
{
class MockPteData : public PteData
{
public:
    using PteData::PteData;
};

} // namespace pbr

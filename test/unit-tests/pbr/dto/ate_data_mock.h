#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/dto/ate_data.h"

namespace pbr
{
class MockAteData : public AteData
{
public:
    using AteData::AteData;
};

} // namespace pbr

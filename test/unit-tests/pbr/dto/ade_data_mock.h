#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/dto/ade_data.h"

namespace pbr
{
class MockAdeData : public AdeData
{
public:
    using AdeData::AdeData;
};

} // namespace pbr

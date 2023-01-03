#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/checker/crc32.h"

namespace pbr
{
class MockCrc32 : public Crc32
{
public:
    using Crc32::Crc32;
    MOCK_METHOD(uint32_t, Make, (char* data, uint32_t length), (override));
};

} // namespace pbr

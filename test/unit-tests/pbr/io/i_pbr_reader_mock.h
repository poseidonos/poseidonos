#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/io/i_pbr_reader.h"

namespace pbr
{
class MockIPbrReader : public IPbrReader
{
public:
    using IPbrReader::IPbrReader;
    MOCK_METHOD(int, Read, (pos::UblockSharedPtr dev, char* dataOut, uint64_t startLba, uint32_t length), (override));
    MOCK_METHOD(int, Read, (string filePath, char* dataOut, uint64_t startOffset, uint32_t length), (override));
};

} // namespace pbr

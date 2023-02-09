#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/io/pbr_reader.h"

namespace pbr
{
class MockPbrReader : public PbrReader
{
public:
    using PbrReader::PbrReader;
    MOCK_METHOD(int, Read, (pos::UblockSharedPtr dev, char* dataOut, uint64_t startLba, uint32_t length), (override));
    MOCK_METHOD(int, Read, (string filePath, char* dataOut, uint64_t startOffset, uint32_t length), (override));
};

} // namespace pbr

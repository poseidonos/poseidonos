#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/io/pbr_writer.h"

namespace pbr
{
class MockPbrWriter : public PbrWriter
{
public:
    using PbrWriter::PbrWriter;
    MOCK_METHOD(int, Write, (pos::UblockSharedPtr dev, char* data, uint64_t startLba, uint32_t length), (override));
    MOCK_METHOD(int, Write, (string filePath, char* data, uint64_t startOffset, uint32_t length), (override));
};

} // namespace pbr

#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/io/i_pbr_writer.h"

namespace pbr
{
class MockIPbrWriter : public IPbrWriter
{
public:
    using IPbrWriter::IPbrWriter;
    MOCK_METHOD(int, Write, (pos::UblockSharedPtr dev, char* data, uint64_t startLba, uint32_t length), (override));
    MOCK_METHOD(int, Write, (string filePath, char* data, uint64_t startOffset, uint32_t length), (override));
};

} // namespace pbr

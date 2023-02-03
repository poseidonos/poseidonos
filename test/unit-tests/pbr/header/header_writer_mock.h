#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/header/header_writer.h"

namespace pbr
{
class MockHeaderWriter : public HeaderWriter
{
public:
    using HeaderWriter::HeaderWriter;
    MOCK_METHOD(int, Write, (HeaderElement* pHeader, pos::UblockSharedPtr dev), (override));
    MOCK_METHOD(int, Write, (HeaderElement* pHeader, string filePath), (override));
};

} // namespace pbr

#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/header/i_header_writer.h"

namespace pbr
{
class MockIHeaderWriter : public IHeaderWriter
{
public:
    using IHeaderWriter::IHeaderWriter;
    MOCK_METHOD(int, Write, (HeaderElement* pHeader, pos::UblockSharedPtr dev), (override));
    MOCK_METHOD(int, Write, (HeaderElement* pHeader, string filePath), (override));
};

} // namespace pbr

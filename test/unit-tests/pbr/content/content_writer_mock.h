#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/content/content_writer.h"

namespace pbr
{
class MockContentWriter : public ContentWriter
{
public:
    using ContentWriter::ContentWriter;
    MOCK_METHOD(int, Write, (AteData* content, pos::UblockSharedPtr dev), (override));
    MOCK_METHOD(int, Write, (AteData* content, string filePath), (override));
};

} // namespace pbr

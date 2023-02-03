#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/content/i_content_writer.h"

namespace pbr
{
class MockIContentWriter : public IContentWriter
{
public:
    using IContentWriter::IContentWriter;
    MOCK_METHOD(int, Write, (AteData* content, pos::UblockSharedPtr dev), (override));
    MOCK_METHOD(int, Write, (AteData* content, string filePath), (override));
};

} // namespace pbr

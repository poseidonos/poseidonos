#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/content/i_content_serializer.h"

namespace pbr
{
class MockIContentSerializer : public IContentSerializer
{
public:
    using IContentSerializer::IContentSerializer;
    MOCK_METHOD(int, Serialize, (char* rawDataOut, AteData* ateData), (override));
    MOCK_METHOD(int, Deserialize, (AteData* out, char* rawData), (override));
    MOCK_METHOD(uint32_t, GetContentSize, (), (override));
    MOCK_METHOD(uint64_t, GetContentStartLba, (), (override));
};

} // namespace pbr

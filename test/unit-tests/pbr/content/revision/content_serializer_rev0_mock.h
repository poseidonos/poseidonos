#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/content/revision0/content_serializer_rev0.h"

namespace pbr
{
class MockContentSerializerRev0 : public ContentSerializerRev0
{
public:
    using ContentSerializerRev0::ContentSerializerRev0;
    MOCK_METHOD(int, Serialize, (char* dataOut, AteData* ateData), (override));
    MOCK_METHOD(int, Deserialize, (AteData* out, char* rawData), (override));
    MOCK_METHOD(uint32_t, GetContentSize, (), (override));
    MOCK_METHOD(uint64_t, GetContentStartLba, (), (override));
};

} // namespace pbr

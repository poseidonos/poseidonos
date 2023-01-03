#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/header/header_serializer.h"

namespace pbr
{
class MockHeaderSerializer : public HeaderSerializer
{
public:
    using HeaderSerializer::HeaderSerializer;
    MOCK_METHOD(int, Serialize, (HeaderElement* pHeader, char* dataOut, uint32_t length), (override));
    MOCK_METHOD(int, Deserialize, (char* rawData, uint32_t length, HeaderElement* pHeaderOut), (override));
};

} // namespace pbr

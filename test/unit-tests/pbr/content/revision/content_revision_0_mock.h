#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/content/revision/content_revision_0.h"

namespace pbr
{
class MockContentRevision_0 : public ContentRevision_0
{
public:
    using ContentRevision_0::ContentRevision_0;
    MOCK_METHOD(int, Serialize, (char* dataOut, AteData* ateData), (override));
    MOCK_METHOD(int, Deserialize, (AteData* out, char* rawData), (override));
    MOCK_METHOD(uint32_t, GetRevision, (), (override));
    MOCK_METHOD(uint32_t, GetContentSize, (), (override));
    MOCK_METHOD(uint64_t, GetContentStartLba, (), (override));
};

} // namespace pbr

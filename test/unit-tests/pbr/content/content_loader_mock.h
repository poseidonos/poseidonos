#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/content/content_loader.h"

namespace pbr
{
class MockContentLoader : public ContentLoader
{
public:
    using ContentLoader::ContentLoader;
    MOCK_METHOD(int, Load, (AteData* out, pos::UblockSharedPtr dev), (override));
    MOCK_METHOD(int, Load, (AteData* out, string filePath), (override));
};

} // namespace pbr

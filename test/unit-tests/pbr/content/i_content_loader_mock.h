#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/content/i_content_loader.h"

namespace pbr
{
class MockIContentLoader : public IContentLoader
{
public:
    using IContentLoader::IContentLoader;
    MOCK_METHOD(int, Load, (AteData* out, pos::UblockSharedPtr dev), (override));
    MOCK_METHOD(int, Load, (AteData* out, string filePath), (override));
};

} // namespace pbr

#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/header/header_loader.h"

namespace pbr
{
class MockHeaderLoader : public HeaderLoader
{
public:
    using HeaderLoader::HeaderLoader;
    MOCK_METHOD(int, Load, (HeaderElement* pHeaderOut, pos::UblockSharedPtr dev), (override));
    MOCK_METHOD(int, Load, (HeaderElement* pHeaderOut, string filePath), (override));
};

} // namespace pbr

#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/header/i_header_loader.h"

namespace pbr
{
class MockIHeaderLoader : public IHeaderLoader
{
public:
    using IHeaderLoader::IHeaderLoader;
    MOCK_METHOD(int, Load, (HeaderElement* pHeaderOut, pos::UblockSharedPtr dev), (override));
    MOCK_METHOD(int, Load, (HeaderElement* pHeaderOut, string filePath), (override));
};

} // namespace pbr

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/master_context/version_provider.h"

namespace pos
{
class MockVersionProvider : public VersionProvider
{
public:
    using VersionProvider::VersionProvider;
    MOCK_METHOD(const std::string, GetVersion, (), (override));
};

} // namespace pos

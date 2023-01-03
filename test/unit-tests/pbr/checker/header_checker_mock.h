#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/checker/header_checker.h"

namespace pbr
{
class MockHeaderChecker : public HeaderChecker
{
public:
    using HeaderChecker::HeaderChecker;
    MOCK_METHOD(bool, Check, (HeaderElement* header), (override));
    MOCK_METHOD(int, UpdateChecksum, (HeaderElement* header), (override));
};

} // namespace pbr

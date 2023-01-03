#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/checker/content_checker.h"

namespace pbr
{
class MockContentChecker : public ContentChecker
{
public:
    using ContentChecker::ContentChecker;
    MOCK_METHOD(bool, Check, (AteData* ateData), (override));
    MOCK_METHOD(int, UpdateChecksum, (AteData* ateData), (override));
};

} // namespace pbr

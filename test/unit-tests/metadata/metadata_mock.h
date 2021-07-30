#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metadata/metadata.h"

namespace pos
{
class MockMetadata : public Metadata
{
public:
    using Metadata::Metadata;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, Flush, (), (override));
    MOCK_METHOD(bool, NeedRebuildAgain, (), (override));
    MOCK_METHOD(int, PrepareRebuild, (), (override));
    MOCK_METHOD(void, StopRebuilding, (), (override));
};

} // namespace pos

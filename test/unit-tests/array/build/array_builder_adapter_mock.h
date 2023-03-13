#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/build/array_builder_adapter.h"

namespace pos
{
class MockArrayBuilderAdapter : public ArrayBuilderAdapter
{
public:
    using ArrayBuilderAdapter::ArrayBuilderAdapter;
    MOCK_METHOD(int, Load, (pbr::AteData* ateData, unique_ptr<ArrayBuildInfo>& buildInfo), (override));
    MOCK_METHOD(int, Create, (string name, const DeviceSet<string>& devs,
        string metaRaid, string dataRaid, unique_ptr<ArrayBuildInfo>& buildInfo), (override));
};
} // namespace pos

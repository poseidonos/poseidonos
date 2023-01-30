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
    MOCK_METHOD(ArrayBuildInfo*, Load, (pbr::AteData* ateData), (override));
    MOCK_METHOD(ArrayBuildInfo*, Create, (string name, const DeviceSet<string>& devs,
        string metaRaid, string dataRaid), (override));
};
} // namespace pos

#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/build/array_build_info.h"

namespace pos
{
class MockArrayBuildInfo : public ArrayBuildInfo
{
public:
    using ArrayBuildInfo::ArrayBuildInfo;
};

} // namespace pos

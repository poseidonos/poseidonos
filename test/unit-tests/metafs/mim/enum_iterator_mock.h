#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/enum_iterator.h"

namespace pos
{
template<typename T>
class MockEnum : public Enum<T>
{
public:
    using Enum::Enum;
};

} // namespace pos

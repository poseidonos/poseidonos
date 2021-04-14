#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/common/metafs_type.h"

namespace pos
{
template<typename EnumType>
class MockEnumTypeHash : public EnumTypeHash<EnumType>
{
public:
    using EnumTypeHash::EnumTypeHash;
};

} // namespace pos

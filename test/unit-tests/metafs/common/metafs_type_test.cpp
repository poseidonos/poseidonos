#include "src/metafs/common/metafs_type.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(PairHash, CheckFunction)
{
    std::unordered_map<
        std::pair<int, FileDescriptorType>,
        std::string, PairHash> map;

    map.insert({{1, 10}, "test1"});
    map.insert({{2, 10}, "test2"});
    map.insert({{3, 10}, "test3"});
}

} // namespace pos

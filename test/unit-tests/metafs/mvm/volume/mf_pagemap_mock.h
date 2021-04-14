#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/mf_pagemap.h"

namespace pos
{
class MockMetaFilePageMap : public MetaFilePageMap
{
public:
    using MetaFilePageMap::MetaFilePageMap;
};

} // namespace pos

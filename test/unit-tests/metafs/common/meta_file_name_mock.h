#include <gmock/gmock.h>

#include "src/metafs/common/meta_file_name.h"

class MockMetaFileName : public MetaFileName
{
public:
    using MetaFileName::MetaFileName;
};

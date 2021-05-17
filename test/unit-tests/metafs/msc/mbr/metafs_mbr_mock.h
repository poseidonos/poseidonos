#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/msc/mbr/metafs_mbr.h"

namespace pos
{
class MockMetaFsMBRContent : public MetaFsMBRContent
{
public:
    using MetaFsMBRContent::MetaFsMBRContent;
};

class MockMetaFsMBR : public MetaFsMBR
{
public:
    using MetaFsMBR::MetaFsMBR;
    MOCK_METHOD(bool, GetPORStatus, (), (override));
    MOCK_METHOD(void, SetPORStatus, (bool isShutdownOff), (override));
    MOCK_METHOD(void, InvalidMBRSignature, (), (override));
};

} // namespace pos

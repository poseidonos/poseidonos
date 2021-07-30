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
    MOCK_METHOD(bool, Load, (), (override));
    MOCK_METHOD(bool, Store, (), (override));
    MOCK_METHOD(void, ResetContent, (), (override));
    MOCK_METHOD(const MetaLpnType, GetLpnCntOfRegion, (), (override));
    MOCK_METHOD(void, CreateMBR, ());
    MOCK_METHOD(void, SetMss, (MetaStorageSubsystem* mss));
    MOCK_METHOD(uint64_t, GetEpochSignature, ());
    MOCK_METHOD(bool, IsValidMBRExist, ());
};

} // namespace pos

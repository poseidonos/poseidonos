#include "src/mapper/vsamap/vsamap_manager.h"

#include <gtest/gtest.h>

#include "src/mapper/address/mapper_address_info.h"
#include "src/mapper/vsamap/vsamap_content.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/mapper/address/mapper_address_info_mock.h"
#include "test/unit-tests/mapper/vsamap/vsamap_content_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(VSAMapManager, CreateVsaMapContent_TestFail0)
{
    MapperAddressInfo info;
    info.SetMPageSize(10);
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, &info);
    EXPECT_CALL(*vcon, InMemoryInit).WillOnce(Return(0));
    EXPECT_CALL(*vcon, OpenMapFile).WillOnce(Return(-1));
    bool ret = vsaMap->CreateVsaMapContent(0, 100, false);
    EXPECT_EQ(-1, ret);
    
    delete vsaMap;
}

TEST(VSAMapManager, CreateVsaMapContent_TestFail0)
{
    MapperAddressInfo info;
    info.SetMPageSize(10);
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, &info);
    EXPECT_CALL(*vcon, InMemoryInit).WillOnce(Return(0));
    EXPECT_CALL(*vcon, OpenMapFile).WillOnce(Return(-1));
    bool ret = vsaMap->CreateVsaMapContent(0, 100, false);
    EXPECT_EQ(-1, ret);
    
    delete vsaMap;
}

TEST(VSAMapManager, CreateVsaMapContent_TestFail1)
{
    MapperAddressInfo info;
    info.SetMPageSize(10);
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, &info);
    EXPECT_CALL(*vcon, InMemoryInit).WillOnce(Return(-1));
    bool ret = vsaMap->CreateVsaMapContent(0, 100, false);
    EXPECT_EQ(-1, ret);
    
    delete vsaMap;
}

TEST(VSAMapManager, CreateVsaMapContent_TestFail2)
{
    MapperAddressInfo info;
    info.SetMPageSize(10);
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, &info);
    EXPECT_CALL(*vcon, InMemoryInit).WillOnce(Return(0));
    EXPECT_CALL(*vcon, OpenMapFile).WillOnce(Return(EID(NEED_TO_INITIAL_STORE)));
    EXPECT_CALL(*vcon, FlushHeader).WillOnce(Return(-1));
    bool ret = vsaMap->CreateVsaMapContent(0, 100, false);
    EXPECT_EQ(-1, ret);
    
    delete vsaMap;
}

TEST(VSAMapManager, CreateVsaMapContent_TestFail3)
{
    MapperAddressInfo info;
    info.SetMPageSize(10);
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, &info);
    EXPECT_CALL(*vcon, InMemoryInit).WillOnce(Return(0));
    EXPECT_CALL(*vcon, OpenMapFile).WillOnce(Return(-1));
    bool ret = vsaMap->CreateVsaMapContent(0, 100, false);
    EXPECT_EQ(-1, ret);
    
    delete vsaMap;
}

TEST(VSAMapManager, GetVSAMapContent_TestSimpleGetter)
{
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, nullptr);
    VSAMapContent* ret = vsaMap->GetVSAMapContent(0);
    EXPECT_EQ(vcon, ret);
    delete vsaMap;
}

TEST(VSAMapManager, GetVSAMapContent_TestSimpleSetter)
{
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    NiceMock<MockVSAMapContent>* vcon2 = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, nullptr, nullptr);
    vsaMap->SetVSAMapContent(0, vcon);
    vsaMap->SetVSAMapContent(0, vcon2);
    delete vsaMap;
}

TEST(VSAMapManager, GetVSAMapContent_TestSimpleCaller)
{
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, nullptr);

    vsaMap->EnableVsaMapAccess(0);
    bool ret = vsaMap->IsVsaMapAccessible(0);
    EXPECT_EQ(true, ret);

    vsaMap->EnableVsaMapInternalAccess(0);
    ret = vsaMap->IsVsaMapInternalAccesible(0);
    EXPECT_EQ(true, ret);
    delete vsaMap;
}

TEST(VSAMapManager, GetVSAMapContent_TestFailSetVSA)
{
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, nullptr);

    vsaMap->DisableVsaMapAccess(0);
    VirtualBlks vv;
    int ret = vsaMap->SetVSAs(0, 0, vv);
    EXPECT_EQ(-EID(VSAMAP_NOT_ACCESSIBLE), ret);
    delete vsaMap;
}

TEST(VSAMapManager, GetVSAs_TestFailGetVSA)
{
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, nullptr);

    vsaMap->DisableVsaMapAccess(0);
    VirtualBlkAddr vsa;
    vsa.offset = 0;
    vsa.stripeId = 0;
    VsaArray vv;
    vv[0] = vsa;
    int ret = vsaMap->GetVSAs(0, 0, 1, vv);
    EXPECT_EQ(-EID(VSAMAP_NOT_ACCESSIBLE), ret);
    delete vsaMap;
}

TEST(VSAMapManager, WaitPendingIo_TestSimpleCaller)
{
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, nullptr);

    vsaMap->WaitLoadPendingIoDone();
    vsaMap->WaitWritePendingIoDone();
    delete vsaMap;
}

TEST(VSAMapManager, NeedToDeleteFile_TestFailcase)
{
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, nullptr);
    EXPECT_CALL(*vcon, DoesFileExist).WillOnce(Return(false));
    bool ret = vsaMap->NeedToDeleteFile(0);
    EXPECT_EQ(false, ret);
    
    delete vsaMap;
}

TEST(VSAMapManager, LoadVSAMapFile_TestFail)
{
    MapperAddressInfo info;
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, &info);
    EXPECT_CALL(*vcon, Load).WillOnce(Return(-1));
    MpageList d;
    bool ret = vsaMap->FlushDirtyPagesGiven(0, d, nullptr);
    EXPECT_EQ(-EID(MAP_FLUSH_IN_PROGRESS), ret);
    delete vsaMap;
}

TEST(VSAMapManager, FlushTouchedPages_TestSuccess)
{
    MapperAddressInfo info;
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, &info);
    EXPECT_CALL(*vcon, FlushTouchedPages).WillOnce(Return(0));
    bool ret = vsaMap->FlushAllMaps();
    EXPECT_EQ(-1, ret);
    
    delete vsaMap;
}

TEST(VSAMapManager, FlushTouchedPages_TestFail)
{
    MapperAddressInfo info;
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, &info);
    EXPECT_CALL(*vcon, FlushTouchedPages).WillOnce(Return(-1));
    bool ret = vsaMap->FlushAllMaps();
    EXPECT_EQ(-1, ret);
    
    delete vsaMap;
}

TEST(VSAMapManager, FlushDirtyPagesGiven_TestFail1)
{
    MapperAddressInfo info;
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, &info);
    EXPECT_CALL(*vcon, FlushDirtyPagesGiven).WillOnce(Return(0));
    MpageList d;
    vsaMap->FlushDirtyPagesGiven(0, d, nullptr);
    int ret = vsaMap->FlushDirtyPagesGiven(0, d, nullptr);
    EXPECT_EQ(-EID(MAP_FLUSH_IN_PROGRESS), ret);
    delete vsaMap;
}

TEST(VSAMapManager, FlushDirtyPagesGiven_TestFail2)
{
    MapperAddressInfo info;
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, &info);
    EXPECT_CALL(*vcon, FlushDirtyPagesGiven).WillOnce(Return(-1));
    MpageList d;
    bool ret = vsaMap->FlushDirtyPagesGiven(0, d, nullptr);
    EXPECT_EQ(-EID(MAP_FLUSH_IN_PROGRESS), ret);
    delete vsaMap;
}

TEST(VSAMapManager, FlushAllMaps_TestFailcase)
{
    NiceMock<MockVSAMapContent>* vcon = new NiceMock<MockVSAMapContent>();
    VSAMapManager* vsaMap = new VSAMapManager(nullptr, vcon, nullptr);
    EXPECT_CALL(*vcon, FlushTouchedPages).WillOnce(Return(-1));
    bool ret = vsaMap->FlushAllMaps();
    EXPECT_EQ(-1, ret);
    
    delete vsaMap;
}

} // namespace pos

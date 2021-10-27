#include "src/mapper/mapper_wbt.h"

#include <gtest/gtest.h>

#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/mapper/address/mapper_address_info_mock.h"
#include "test/unit-tests/mapper/reversemap/reversemap_manager_mock.h"
#include "test/unit-tests/mapper/stripemap/stripemap_manager_mock.h"
#include "test/unit-tests/mapper/vsamap/vsamap_manager_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(MapperWbt, GetMapLayout_)
{
    // given
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    MapperAddressInfo addrInfo(arr);
    addrInfo.SetIsUT(true);
    MapperWbt* wbt = new MapperWbt(&addrInfo, nullptr, strMan, revMan);
    std::string fname = "newfile";
    // when
    int ret = wbt->GetMapLayout(fname);
    // then
    EXPECT_EQ(0, ret);
    delete strMan;
    delete revMan;
    delete wbt;
}

TEST(MapperWbt, ReadVsaMap_)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    MapperAddressInfo addrInfo(arr);
    addrInfo.SetIsUT(true);
    MapperWbt* wbt = new MapperWbt(&addrInfo, vsaMan, strMan, revMan);
    std::string fname = "newfile";
    EXPECT_CALL(*vsaMan, Dump).Times(1);
    // when
    int ret = wbt->ReadVsaMap(0, fname);
    // then
    EXPECT_EQ(0, ret);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete wbt;
}

TEST(MapperWbt, ReadVsaMapEntry_)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    MapperAddressInfo addrInfo(arr);
    addrInfo.SetIsUT(true);
    MapperWbt* wbt = new MapperWbt(&addrInfo, vsaMan, strMan, revMan);
    EXPECT_CALL(*vsaMan, GetVSAs).Times(1);
    std::string fname = "newfile";
    // when
    wbt->ReadVsaMapEntry(0, 0, fname);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete wbt;
}

TEST(MapperWbt, WriteVsaMap_)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    MapperAddressInfo addrInfo(arr);
    addrInfo.SetIsUT(true);
    MapperWbt* wbt = new MapperWbt(&addrInfo, vsaMan, strMan, revMan);
    std::string fname = "newfile";
    EXPECT_CALL(*vsaMan, DumpLoad).Times(1);

    // when
    int ret = wbt->WriteVsaMap(0, fname);
    // then
    EXPECT_EQ(0, ret);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete wbt;
}

TEST(MapperWbt, WriteVsaMapEntry_)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    MapperWbt* wbt = new MapperWbt(addrInfo, vsaMan, strMan, revMan);
    VirtualBlkAddr vsa;
    vsa.offset = 0;
    vsa.stripeId = 0;
    EXPECT_CALL(*vsaMan, SetVSAsWoCond).Times(1);
    std::string fname = "newfile";
    // when
    wbt->WriteVsaMapEntry(0, 0, vsa);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete addrInfo;
    delete wbt;
}

TEST(MapperWbt, ReadStripeMap_)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    MapperWbt* wbt = new MapperWbt(addrInfo, vsaMan, strMan, revMan);
    EXPECT_CALL(*strMan, Dump).Times(1);
    std::string fname = "newfile";
    // when
    int ret = wbt->ReadStripeMap(fname);
    // then
    EXPECT_EQ(0, ret);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete addrInfo;
    delete wbt;
}

TEST(MapperWbt, ReadStripeMapEntry_)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    MapperWbt* wbt = new MapperWbt(addrInfo, vsaMan, strMan, revMan);
    EXPECT_CALL(*strMan, GetLSA).Times(1);
    std::string fname = "newfile";
    // when
    int ret = wbt->ReadStripeMapEntry(0, fname);
    // then
    EXPECT_EQ(0, ret);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete addrInfo;
    delete wbt;
}

TEST(MapperWbt, WriteStripeMap_)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    MapperWbt* wbt = new MapperWbt(addrInfo, vsaMan, strMan, revMan);
    EXPECT_CALL(*strMan, DumpLoad).Times(1);
    std::string fname = "newfile";
    // when
    int ret = wbt->WriteStripeMap(fname);
    // then
    EXPECT_EQ(0, ret);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete addrInfo;
    delete wbt;
}

TEST(MapperWbt, WriteStripeMapEntry_)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    MapperWbt* wbt = new MapperWbt(addrInfo, vsaMan, strMan, revMan);
    EXPECT_CALL(*strMan, SetLSA).Times(1);
    StripeLoc loc = IN_USER_AREA;
    // when
    int ret = wbt->WriteStripeMapEntry(0, loc, 0);
    // then
    EXPECT_EQ(0, ret);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete addrInfo;
    delete wbt;
}

TEST(MapperWbt, ReadReverseMap_TestFail)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    MapperWbt* wbt = new MapperWbt(addrInfo, vsaMan, strMan, revMan);
    StripeLoc loc = IN_USER_AREA;

    EXPECT_CALL(*revMan, LoadReverseMapForWBT).WillOnce(Return(-1));
    // when
    std::string fname = "newfile";
    int ret = wbt->ReadReverseMap(0, fname);
    // then
    EXPECT_EQ(-1, ret);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete addrInfo;
    delete wbt;
}

TEST(MapperWbt, WriteReverseMap_TestFail)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    MapperWbt* wbt = new MapperWbt(addrInfo, vsaMan, strMan, revMan);
    StripeLoc loc = IN_USER_AREA;

    EXPECT_CALL(*revMan, StoreReverseMapForWBT).WillOnce(Return(-1));
    // when
    std::string fname = "newfile";
    int ret = wbt->WriteReverseMap(0, fname);
    // then
    EXPECT_EQ(-1, ret);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete addrInfo;
    delete wbt;
}

TEST(MapperWbt, ReadWholeReverseMap_TestFail)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    MapperWbt* wbt = new MapperWbt(addrInfo, vsaMan, strMan, revMan);
    StripeLoc loc = IN_USER_AREA;

    EXPECT_CALL(*revMan, GetWholeReverseMapFileSize).WillOnce(Return(10));
    EXPECT_CALL(*revMan, LoadReverseMapForWBT).WillOnce(Return(-1));
    // when
    std::string fname = "newfile";
    int ret = wbt->ReadWholeReverseMap(fname);
    // then
    EXPECT_EQ(-1, ret);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete addrInfo;
    delete wbt;
}

TEST(MapperWbt, WriteWholeReverseMap_TestFail)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    MapperWbt* wbt = new MapperWbt(addrInfo, vsaMan, strMan, revMan);
    StripeLoc loc = IN_USER_AREA;

    EXPECT_CALL(*revMan, GetWholeReverseMapFileSize).WillOnce(Return(10));
    EXPECT_CALL(*revMan, StoreReverseMapForWBT).WillOnce(Return(-1));
    // when
    std::string fname = "newfile";
    int ret = wbt->WriteWholeReverseMap(fname);
    // then
    EXPECT_EQ(-1, ret);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete addrInfo;
    delete wbt;
}

TEST(MapperWbt, ReadReverseMapEntry_TestFail)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    MapperWbt* wbt = new MapperWbt(addrInfo, vsaMan, strMan, revMan);
    StripeLoc loc = IN_USER_AREA;

    EXPECT_CALL(*revMan, GetReverseMapPerStripeFileSize).WillOnce(Return(10));
    EXPECT_CALL(*revMan, LoadReverseMapForWBT).WillOnce(Return(-1));
    // when
    std::string fname = "newfile";
    int ret = wbt->ReadReverseMapEntry(0, 0, fname);
    // then
    EXPECT_EQ(-1, ret);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete addrInfo;
    delete wbt;
}

TEST(MapperWbt, WriteReverseMapEntry_TestFail0)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    MapperWbt* wbt = new MapperWbt(addrInfo, vsaMan, strMan, revMan);
    StripeLoc loc = IN_USER_AREA;

    EXPECT_CALL(*revMan, GetReverseMapPerStripeFileSize).WillOnce(Return(10));
    EXPECT_CALL(*revMan, LoadReverseMapForWBT).WillOnce(Return(-1));
    // when
    int ret = wbt->WriteReverseMapEntry(0, 0, 0, 0);
    // then
    EXPECT_EQ(-1, ret);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete addrInfo;
    delete wbt;
}

TEST(MapperWbt, WriteReverseMapEntry_TestFail1)
{
    // given
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    MapperWbt* wbt = new MapperWbt(addrInfo, vsaMan, strMan, revMan);
    StripeLoc loc = IN_USER_AREA;

    EXPECT_CALL(*revMan, GetReverseMapPerStripeFileSize).WillOnce(Return(10));
    EXPECT_CALL(*revMan, LoadReverseMapForWBT).WillOnce(Return(0)).WillOnce(Return(-1));
    // when
    int ret = wbt->WriteReverseMapEntry(0, 0, 0, 0);
    // then
    EXPECT_EQ(-1, ret);
    delete vsaMan;
    delete strMan;
    delete revMan;
    delete addrInfo;
    delete wbt;
}

} // namespace pos

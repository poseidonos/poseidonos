#include "src/mbr/mbr_map_manager.h"

#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"

namespace pos
{
TEST(MbrMapManager, MbrMapManager_testMbrMapaManagerCreate)
{
    // Given : Nothing
    // When : new MbrMap Manager
    MbrMapManager* mmMgr = new MbrMapManager;
    // Then : Nothing
}

TEST(MbrMapManager, InsertDevices_testInsertingDeviceInfoAsMeta)
{
    // Given : empty deviceIndexMap
    MbrMapManager* mmMgr = new MbrMapManager;
    string mockDeviceUid0 = "unvme-ns-0";
    string mockDeviceUid1 = "unvme-ns-1";
    string mockDeviceUid2 = "unvme-ns-2";
    string mockDeviceUid3 = "unvme-ns-3";
    int mockArrayIndex = 0;

    // When : Insert mockDevice
    ArrayMeta meta;
    meta.devs.data.push_back(DeviceMeta(mockDeviceUid0));
    meta.devs.data.push_back(DeviceMeta(mockDeviceUid1));
    meta.devs.data.push_back(DeviceMeta(mockDeviceUid2));
    meta.devs.spares.push_back(DeviceMeta(mockDeviceUid3));
    mmMgr->InsertDevices(meta, mockArrayIndex);

    // Then : mockDevice information added to map
    int result = mmMgr->CheckAllDevices(meta);
    EXPECT_EQ(EID(MBR_DEVICE_ALREADY_IN_ARRAY), result);
}

TEST(MbrMapManager, InsertDevice_testInsertingDeviceInfo)
{
    // Given : empty deviceIndexMap
    MbrMapManager* mmMgr = new MbrMapManager;
    string mockDeviceUid = "unvme-ns-0";
    int mockArrayIndex = 0;

    // When : Insert mockDevice
    mmMgr->InsertDevice(mockDeviceUid, mockArrayIndex);

    // Then : mockDevice information added to map
    ArrayMeta meta;
    meta.devs.data.push_back(DeviceMeta(mockDeviceUid));
    int result = mmMgr->CheckAllDevices(meta);
    EXPECT_EQ(EID(MBR_DEVICE_ALREADY_IN_ARRAY), result);
}

TEST(MbrMapManager, DeleteDevices_testDeletingDeviceInfo)
{
    // Given : deviceIndexMap with one device info
    MbrMapManager* mmMgr = new MbrMapManager;
    string mockDeviceUid = "unvme-ns-0";
    int mockArrayIndex = 0;
    mmMgr->InsertDevice(mockDeviceUid, mockArrayIndex);

    ArrayMeta meta;
    meta.devs.data.push_back(DeviceMeta(mockDeviceUid));
    int result = mmMgr->CheckAllDevices(meta);
    EXPECT_EQ(EID(MBR_DEVICE_ALREADY_IN_ARRAY), result);
    // When : Delete mockDevices
    mmMgr->DeleteDevices(mockArrayIndex);
    // Then : mockDevice information added to map
    result = mmMgr->CheckAllDevices(meta);
    EXPECT_EQ(0, result);
}

TEST(MbrMapManager, ResetMap_testResettingMap)
{
    // Given : deviceIndexMap with one device info
    MbrMapManager* mmMgr = new MbrMapManager;
    string mockDeviceUid = "unvme-ns-0";
    int mockArrayIndex = 0;
    mmMgr->InsertDevice(mockDeviceUid, mockArrayIndex);

    ArrayMeta meta;
    meta.devs.data.push_back(DeviceMeta(mockDeviceUid));
    int result = mmMgr->CheckAllDevices(meta);
    EXPECT_EQ(EID(MBR_DEVICE_ALREADY_IN_ARRAY), result);
    // When : Delete mockDevices
    mmMgr->ResetMap();
    // Then : mockDevice information added to map
    result = mmMgr->CheckAllDevices(meta);
    EXPECT_EQ(0, result);
}

TEST(MbrMapManager, CheckAllDevices_testDeviceInfoWithNoInfoInMap)
{
    // Given : deviceIndexMap with no device info
    MbrMapManager* mmMgr = new MbrMapManager;
    string mockDeviceUid = "unvme-ns-0";
    int mockArrayIndex = 0;

    ArrayMeta meta;
    meta.devs.data.push_back(DeviceMeta(mockDeviceUid));
    // When : Check mockDevices
    int result = mmMgr->CheckAllDevices(meta);
    // Then : Device Not exist
    EXPECT_EQ(0, result);
}

TEST(MbrMapManager, CheckAllDevices_testDeviceInfoWithOneInfoInMap)
{
    // Given : deviceIndexMap with one device info
    MbrMapManager* mmMgr = new MbrMapManager;
    string mockDeviceUid = "unvme-ns-0";
    int mockArrayIndex = 0;
    mmMgr->InsertDevice(mockDeviceUid, mockArrayIndex);

    ArrayMeta meta;
    meta.devs.data.push_back(DeviceMeta(mockDeviceUid));
    // When : Check mockDevices
    int result = mmMgr->CheckAllDevices(meta);
    // Then : Device already exist
    EXPECT_EQ(EID(MBR_DEVICE_ALREADY_IN_ARRAY), result);
}

TEST(MbrMapManager, FindArrayIndex_testFindingArrayIndexWithOneInfoInMap)
{
    // Given : deviceIndexMap with on deivce info
    MbrMapManager* mmMgr = new MbrMapManager;
    string mockDeviceUid = "unvme-ns-0";
    int mockArrayIndex = 0;
    mmMgr->InsertDevice(mockDeviceUid, mockArrayIndex);

    // When : Find array index with device name
    int result = mmMgr->FindArrayIndex(mockDeviceUid);

    // Then : Result array index is equal to info
    EXPECT_EQ(mockArrayIndex, result);
}

TEST(MbrMapManager, FindArrayIndex_testFindingArrayIndexWithNoInfoInMap)
{
    // Given : deviceIndexMap with on deivce info
    MbrMapManager* mmMgr = new MbrMapManager;
    string mockDeviceUid = "unvme-ns-0";
    int failedResult = -1;

    // When : Find array index with device name
    int result = mmMgr->FindArrayIndex(mockDeviceUid);

    // Then : Result array index is equal to info
    EXPECT_EQ(failedResult, result);
}

} // namespace pos

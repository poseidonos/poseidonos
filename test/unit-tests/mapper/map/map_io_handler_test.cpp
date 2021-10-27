#include "src/mapper/map/map_io_handler.h"

#include <gtest/gtest.h>

#include "src/mapper/address/mapper_address_info.h"
#include "test/unit-tests/allocator/block_manager/block_manager_mock.h"
#include "test/unit-tests/io/frontend_io/flush_command_manager_mock.h"
#include "test/unit-tests/mapper/address/mapper_address_info_mock.h"
#include "test/unit-tests/mapper/map/map_header_mock.h"
#include "test/unit-tests/meta_file_intf/meta_file_intf_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(MapIoHandler, MapIoHandler_)
{
    MapIoHandler mio(nullptr, nullptr, nullptr, 0, nullptr);
}

TEST(MapIoHandler, OpenFile_TestFailCase)
{
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("s", 1);
    MapIoHandler mio(file, nullptr, nullptr, 0, &addrInfo);
    EXPECT_CALL(addrInfo, IsUT).WillOnce(Return(true));
    EXPECT_CALL(addrInfo, GetArrayId).WillOnce(Return(0));
    EXPECT_CALL(*file, Create).WillOnce(Return(-1));
    int ret = mio.OpenFile("s", 0);
    EXPECT_EQ(-1, ret);
}

TEST(MapIoHandler, DeleteFile_TestFail)
{
    MapperAddressInfo addrInfo;
    addrInfo.SetIsUT(true);
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("s", 1);
    MapIoHandler mio(file, nullptr, nullptr, 0, &addrInfo);
    EXPECT_CALL(*file, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*file, IsOpened).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_CALL(*file, Delete).WillOnce(Return(-1));
    int ret = mio.DeleteFile();
    EXPECT_EQ(-1, ret);
}

TEST(MapIoHandler, DoesFileExist_TestFail)
{
    MapperAddressInfo addrInfo;
    addrInfo.SetIsUT(true);
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("s", 1);
    MapIoHandler mio(file, nullptr, nullptr, 0, &addrInfo);
    bool ret = mio.DoesFileExist();
    EXPECT_EQ(false, ret);
}

TEST(MapIoHandler, Load_TestFail0)
{
    MapperAddressInfo addrInfo;
    addrInfo.SetIsUT(true);
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("s", 1);
    MapIoHandler mio(file, nullptr, nullptr, 0, &addrInfo);
    EXPECT_CALL(*file, DoesFileExist).WillOnce(Return(false));
    AsyncLoadCallBack cb;
    int ret = mio.Load(cb);
    EXPECT_EQ(-1, ret);
}

TEST(MapIoHandler, Load_TestFail1)
{
    MapperAddressInfo addrInfo;
    addrInfo.SetIsUT(true);
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("s", 1);
    NiceMock<MockMapHeader>* header = new NiceMock<MockMapHeader>(0);
    MapIoHandler mio(file, nullptr, header, 0, &addrInfo);
    EXPECT_CALL(*file, DoesFileExist).WillOnce(Return(true));
    ON_CALL(*file, IsOpened).WillByDefault(Return(false));
    EXPECT_CALL(*file, Open).Times(1);
    EXPECT_CALL(*header, GetSize).WillOnce(Return(50));
    EXPECT_CALL(*file, AsyncIO).WillOnce(Return(-1));
    AsyncLoadCallBack cb;
    int ret = mio.Load(cb);
    EXPECT_EQ(-1, ret);

    delete header;
}

} // namespace pos

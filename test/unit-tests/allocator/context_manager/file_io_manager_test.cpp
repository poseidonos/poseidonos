#include "src/allocator/context_manager/file_io_manager.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/meta_file_intf/meta_file_intf_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(AllocatorFileIoManager, AllocatorFileIoManager_)
{
}

TEST(AllocatorFileIoManager, Init_TestInitAndClose)
{
    // given
    NiceMock<AllocatorAddressInfo>* addrInfo = new NiceMock<AllocatorAddressInfo>();
    NiceMock<MockMetaFileIntf>* file[NUM_FILES];
    for (int i = 0; i < NUM_FILES; i++)
    {
        file[i] = new NiceMock<MockMetaFileIntf>("aa", "bb");
    }
    AllocatorFileIoManager fileManager((MetaFileIntf**)file, addrInfo, "");
    // when 1.
    fileManager.Init();
    // given 1.
    EXPECT_CALL(*file[0], IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*file[1], IsOpened).WillOnce(Return(false));
    EXPECT_CALL(*file[0], Close);
    fileManager.Dispose();
    // when 2.
    fileManager.Dispose();
}

TEST(AllocatorFileIoManager, UpdateSectionInfo_TestSimpleSetter)
{
    // given
    NiceMock<AllocatorAddressInfo>* addrInfo = new NiceMock<AllocatorAddressInfo>();
    NiceMock<MockMetaFileIntf>* file[NUM_FILES];
    for (int i = 0; i < NUM_FILES; i++)
    {
        file[i] = new NiceMock<MockMetaFileIntf>("aa", "bb");
    }
    AllocatorFileIoManager fileManager((MetaFileIntf**)file, addrInfo, "");
    // when
    fileManager.UpdateSectionInfo(0, 0, nullptr, 0, 0);
    for (int i = 0; i < NUM_FILES; i++)
    {
        delete file[i];
    }
}

TEST(AllocatorFileIoManager, LoadSync_TestFileLoadWhenFileExistOrNot)
{
    // given
    NiceMock<AllocatorAddressInfo>* addrInfo = new NiceMock<AllocatorAddressInfo>();
    NiceMock<MockMetaFileIntf>* file[NUM_FILES];
    for (int i = 0; i < NUM_FILES; i++)
    {
        file[i] = new NiceMock<MockMetaFileIntf>("aa", "bb");
    }
    AllocatorFileIoManager fileManager((MetaFileIntf**)file, addrInfo, "");
    char* buf = new char[100];
    // given 1.
    EXPECT_CALL(*file[0], DoesFileExist).WillOnce(Return(false));
    EXPECT_CALL(*file[0], Create).WillOnce(Return(-1));
    // when 1.
    int ret = fileManager.LoadSync(0, buf);
    // then 1.
    EXPECT_EQ(-1, ret);

    // given 2.
    EXPECT_CALL(*file[0], DoesFileExist).WillOnce(Return(false));
    EXPECT_CALL(*file[0], Create).WillOnce(Return(0));
    // when 2.
    ret = fileManager.LoadSync(0, buf);
    // then 2.
    EXPECT_EQ(0, ret);

    // given 3.
    EXPECT_CALL(*file[0], DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*file[0], IssueIO).WillOnce(Return(-1));
    // when 3.
    ret = fileManager.LoadSync(0, buf);
    // then 3.
    EXPECT_EQ(-1, ret);

    // given 4.
    EXPECT_CALL(*file[0], DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*file[0], IssueIO).WillOnce(Return(0));
    // when 3.
    ret = fileManager.LoadSync(0, buf);
    // then 3.
    EXPECT_EQ(1, ret);
    delete[] buf;
    delete addrInfo;
    for (int i = 0; i < NUM_FILES; i++)
    {
        delete file[i];
    }
}

TEST(AllocatorFileIoManager, StoreSync_TestSimpleCaller)
{
    // given
    NiceMock<AllocatorAddressInfo>* addrInfo = new NiceMock<AllocatorAddressInfo>();
    NiceMock<MockMetaFileIntf>* file[NUM_FILES];
    for (int i = 0; i < NUM_FILES; i++)
    {
        file[i] = new NiceMock<MockMetaFileIntf>("aa", "bb");
    }
    AllocatorFileIoManager fileManager((MetaFileIntf**)file, addrInfo, "");
    char* buf = new char[100];
    EXPECT_CALL(*file[0], IssueIO);
    // when
    fileManager.StoreSync(0, buf);
    delete[] buf;
    delete addrInfo;
    for (int i = 0; i < NUM_FILES; i++)
    {
        delete file[i];
    }
}

TEST(AllocatorFileIoManager, StoreAsync_TestSimpleCaller)
{
    // given
    NiceMock<AllocatorAddressInfo>* addrInfo = new NiceMock<AllocatorAddressInfo>();
    NiceMock<MockMetaFileIntf>* file[NUM_FILES];
    for (int i = 0; i < NUM_FILES; i++)
    {
        file[i] = new NiceMock<MockMetaFileIntf>("aa", "bb");
    }
    AllocatorFileIoManager fileManager((MetaFileIntf**)file, addrInfo, "");
    char* buf = new char[100];
    // given 1.
    EXPECT_CALL(*file[0], GetFd).WillOnce(Return(0));
    EXPECT_CALL(*file[0], AsyncIO).WillOnce(Return(0));
    // when 1.
    fileManager.StoreAsync(0, buf, nullptr);

    // given 2.
    EXPECT_CALL(*file[0], GetFd).WillOnce(Return(0));
    EXPECT_CALL(*file[0], AsyncIO).WillOnce(Return(-2));
    // when 2.
    fileManager.StoreAsync(0, buf, nullptr);

    delete addrInfo;
    for (int i = 0; i < NUM_FILES; i++)
    {
        delete file[i];
    }
    delete[] buf;
}

TEST(AllocatorFileIoManager, LoadSectionData_)
{
    // given
    NiceMock<AllocatorAddressInfo>* addrInfo = new NiceMock<AllocatorAddressInfo>();
    NiceMock<MockMetaFileIntf>* file[NUM_FILES];
    for (int i = 0; i < NUM_FILES; i++)
    {
        file[i] = new NiceMock<MockMetaFileIntf>("aa", "bb");
    }
    AllocatorFileIoManager fileManager((MetaFileIntf**)file, addrInfo, "");
    char bufSrc[100];
    char bufDst[100];
    bufSrc[0] = 'a';
    bufSrc[10] = 'b';
    bufSrc[20] = 'c';
    fileManager.UpdateSectionInfo(0, 0, &bufDst[0], 10, 0);
    fileManager.UpdateSectionInfo(0, 1, &bufDst[10], 10, 10);
    fileManager.UpdateSectionInfo(0, 2, &bufDst[20], 30, 20);
    // when
    fileManager.LoadSectionData(0, bufSrc);
    // then
    EXPECT_EQ(bufSrc[0], bufDst[0]);
    EXPECT_EQ(bufSrc[10], bufDst[10]);
    EXPECT_EQ(bufSrc[20], bufDst[20]);
    delete addrInfo;
    for (int i = 0; i < NUM_FILES; i++)
    {
        delete file[i];
    }
}

TEST(AllocatorFileIoManager, CopySectionData_)
{ // given
    NiceMock<AllocatorAddressInfo>* addrInfo = new NiceMock<AllocatorAddressInfo>();
    NiceMock<MockMetaFileIntf>* file[NUM_FILES];
    for (int i = 0; i < NUM_FILES; i++)
    {
        file[i] = new NiceMock<MockMetaFileIntf>("aa", "bb");
    }
    AllocatorFileIoManager fileManager((MetaFileIntf**)file, addrInfo, "");
    char bufSrc[100];
    char bufDst[100];
    bufSrc[0] = 'a';
    bufSrc[10] = 'b';
    bufSrc[20] = 'c';
    fileManager.UpdateSectionInfo(0, 0, &bufSrc[0], 10, 0);
    fileManager.UpdateSectionInfo(0, 1, &bufSrc[10], 10, 10);
    fileManager.UpdateSectionInfo(0, 2, &bufSrc[20], 30, 20);
    // when
    fileManager.CopySectionData(0, bufDst, 0, 3);
    // then
    EXPECT_EQ(bufSrc[0], bufDst[0]);
    EXPECT_EQ(bufSrc[10], bufDst[10]);
    EXPECT_EQ(bufSrc[20], bufDst[20]);
    delete addrInfo;
    for (int i = 0; i < NUM_FILES; i++)
    {
        delete file[i];
    }
}

TEST(AllocatorFileIoManager, GetFileSize_TestSimpleGetter)
{
    // given
    NiceMock<AllocatorAddressInfo>* addrInfo = new NiceMock<AllocatorAddressInfo>();
    NiceMock<MockMetaFileIntf>* file[NUM_FILES];
    for (int i = 0; i < NUM_FILES; i++)
    {
        file[i] = new NiceMock<MockMetaFileIntf>("aa", "bb");
    }
    AllocatorFileIoManager fileManager((MetaFileIntf**)file, addrInfo, "");
    // when
    int ret = fileManager.GetFileSize(0);
    delete addrInfo;
    for (int i = 0; i < NUM_FILES; i++)
    {
        delete file[i];
    }
}

TEST(AllocatorFileIoManager, GetSectionAddr_TestSimpleGetter)
{
    // given
    NiceMock<AllocatorAddressInfo>* addrInfo = new NiceMock<AllocatorAddressInfo>();
    NiceMock<MockMetaFileIntf>* file[NUM_FILES];
    for (int i = 0; i < NUM_FILES; i++)
    {
        file[i] = new NiceMock<MockMetaFileIntf>("aa", "bb");
    }
    AllocatorFileIoManager fileManager((MetaFileIntf**)file, addrInfo, "");
    fileManager.UpdateSectionInfo(0, 0, (char*)100, 10, 0);
    // when
    char* ret = fileManager.GetSectionAddr(0, 0);
    // then
    EXPECT_EQ((char*)100, ret);
    delete addrInfo;
    for (int i = 0; i < NUM_FILES; i++)
    {
        delete file[i];
    }
}

TEST(AllocatorFileIoManager, GetSectionSize_)
{
    // given
    NiceMock<AllocatorAddressInfo>* addrInfo = new NiceMock<AllocatorAddressInfo>();
    NiceMock<MockMetaFileIntf>* file[NUM_FILES];
    for (int i = 0; i < NUM_FILES; i++)
    {
        file[i] = new NiceMock<MockMetaFileIntf>("aa", "bb");
    }
    AllocatorFileIoManager fileManager((MetaFileIntf**)file, addrInfo, "");
    fileManager.UpdateSectionInfo(0, 0, (char*)100, 10, 0);
    // when
    int ret = fileManager.GetSectionSize(0, 0);
    // then
    EXPECT_EQ(10, ret);
    delete addrInfo;
    for (int i = 0; i < NUM_FILES; i++)
    {
        delete file[i];
    }
}

TEST(AllocatorFileIoManager, GetSectionOffset_)
{
    // given
    NiceMock<AllocatorAddressInfo>* addrInfo = new NiceMock<AllocatorAddressInfo>();
    NiceMock<MockMetaFileIntf>* file[NUM_FILES];
    for (int i = 0; i < NUM_FILES; i++)
    {
        file[i] = new NiceMock<MockMetaFileIntf>("aa", "bb");
    }
    AllocatorFileIoManager fileManager((MetaFileIntf**)file, addrInfo, "");
    fileManager.UpdateSectionInfo(0, 0, (char*)100, 10, 20);
    // when
    int ret = fileManager.GetSectionOffset(0, 0);
    // then
    EXPECT_EQ(20, ret);
    delete addrInfo;
    for (int i = 0; i < NUM_FILES; i++)
    {
        delete file[i];
    }
}

} // namespace pos

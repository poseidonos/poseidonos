#include "src/volume/volume_meta_intf.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "test/unit-tests/metafs/metafs_file_intf_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

using namespace std;

namespace pos
{
class VolumeMetaIntfFixture : public ::testing::Test
{
public:
    VolumeMetaIntfFixture(void)
    {
    }

    virtual ~VolumeMetaIntfFixture(void)
    {
    }

    virtual void SetUp(void)
    {
        file = make_unique<NiceMock<MockMetaFsFileIntf>>(FILE_NAME, ARRAY_ID, FILE_TYPE);
        list.Clear();
    }

    virtual void TearDown(void)
    {
    }

protected:
    const std::string FILE_NAME = "testFile";
    const MetaFileType FILE_TYPE = MetaFileType::Map;
    const std::string ARRAY_NAME = "array";
    const int ARRAY_ID = 1;
    const size_t FILE_SIZE = 4096;

    unique_ptr<NiceMock<MockMetaFsFileIntf>> file;

    VolumeList list;
};

TEST_F(VolumeMetaIntfFixture, LoadVolumes_testIfAnErrorCodeWillBeReturnedWhenTheFileDoesNotExist)
{
    EXPECT_CALL(*file, DoesFileExist()).WillOnce(Return(false));

    int expectValue = EID(VOL_UNABLE_TO_LOAD_OPEN_FAILED);
    EXPECT_EQ(VolumeMetaIntf::LoadVolumes(list, ARRAY_NAME, ARRAY_ID, move(file)), expectValue);
}

TEST_F(VolumeMetaIntfFixture, LoadVolumes_testIfAnErrorCodeWillBeReturnedWhenTheFileCannotBeOpened)
{
    EXPECT_CALL(*file, DoesFileExist()).WillOnce(Return(true));
    EXPECT_CALL(*file, Open()).WillOnce(Return(EID(MFS_FILE_NOT_FOUND)));

    int expectValue = EID(VOL_UNABLE_TO_LOAD_OPEN_FAILED);
    EXPECT_EQ(VolumeMetaIntf::LoadVolumes(list, ARRAY_NAME, ARRAY_ID, move(file)), expectValue);
}

TEST_F(VolumeMetaIntfFixture, LoadVolumes_testIfAnErrorCodeWillBeReturnedWhenReadRequestIsFailed)
{
    EXPECT_CALL(*file, DoesFileExist()).WillOnce(Return(true));
    EXPECT_CALL(*file, Open()).WillOnce(Return(EID(SUCCESS)));
    EXPECT_CALL(*file, AsyncIO).WillOnce([&](auto ctx)
        {
            return EID(MFS_FILE_READ_FAILED);
        });
    EXPECT_CALL(*file, Close()).WillOnce(Return(EID(SUCCESS)));

    int expectValue = EID(MFS_FILE_READ_FAILED);
    EXPECT_EQ(VolumeMetaIntf::LoadVolumes(list, ARRAY_NAME, ARRAY_ID, move(file)), expectValue);
}

TEST_F(VolumeMetaIntfFixture, LoadVolumes_testIfAnErrorCodeWillBeReturnedWhenCloseRequestIsFailed)
{
    EXPECT_CALL(*file, DoesFileExist()).WillOnce(Return(true));
    EXPECT_CALL(*file, Open()).WillOnce(Return(EID(SUCCESS)));
    EXPECT_CALL(*file, AsyncIO).WillOnce([&](auto ctx)
        {
            ctx->HandleIoComplete(ctx);
            return EID(SUCCESS);
        });
    EXPECT_CALL(*file, Close()).WillOnce(Return(EID(MFS_FILE_NOT_OPENED)));

    int expectValue = EID(VOL_UNABLE_TO_CLOSE_FILE);
    EXPECT_EQ(VolumeMetaIntf::LoadVolumes(list, ARRAY_NAME, ARRAY_ID, move(file)), expectValue);
}

TEST_F(VolumeMetaIntfFixture, LoadVolumes_testIfItHasBeenSuccessfullyFinishedToLoadVolumeMetadata)
{
    EXPECT_CALL(*file, DoesFileExist()).WillOnce(Return(true));
    EXPECT_CALL(*file, Open()).WillOnce(Return(EID(SUCCESS)));
    EXPECT_CALL(*file, AsyncIO).WillOnce([&](auto ctx)
        {
            ctx->HandleIoComplete(ctx);
            return EID(SUCCESS);
        });
    EXPECT_CALL(*file, Close()).WillOnce(Return(EID(SUCCESS)));

    int expectValue = EID(SUCCESS);
    EXPECT_EQ(VolumeMetaIntf::LoadVolumes(list, ARRAY_NAME, ARRAY_ID, move(file)), expectValue);
}

TEST_F(VolumeMetaIntfFixture, SaveVolumes_testIfAnErrorCodeWillBeReturnedWhenTheFileCannotBeCreated)
{
    EXPECT_CALL(*file, DoesFileExist()).WillOnce(Return(false));
    EXPECT_CALL(*file, Create).WillOnce(Return(EID(MFS_FILE_NOT_FOUND)));

    int expectValue = EID(VOL_UNABLE_TO_SAVE_CREATION_FAILED);
    EXPECT_EQ(VolumeMetaIntf::SaveVolumes(list, ARRAY_NAME, ARRAY_ID, move(file)), expectValue);
}

TEST_F(VolumeMetaIntfFixture, SaveVolumes_testIfAnErrorCodeWillBeReturnedWhenTheFileCannotBeOpened)
{
    EXPECT_CALL(*file, DoesFileExist()).WillOnce(Return(false));
    EXPECT_CALL(*file, Create).WillOnce(Return(EID(SUCCESS)));
    EXPECT_CALL(*file, Open).WillOnce(Return(EID(MFS_FILE_NOT_FOUND)));

    int expectValue = EID(VOL_UNABLE_TO_SAVE_OPEN_FAILED);
    EXPECT_EQ(VolumeMetaIntf::SaveVolumes(list, ARRAY_NAME, ARRAY_ID, move(file)), expectValue);
}

TEST_F(VolumeMetaIntfFixture, SaveVolumes_testIfAnErrorCodeWillBeReturnedWhenWriteRequestIsFailed)
{
    EXPECT_CALL(*file, DoesFileExist()).WillOnce(Return(false));
    EXPECT_CALL(*file, Create).WillOnce(Return(EID(SUCCESS)));
    EXPECT_CALL(*file, Open).WillOnce(Return(EID(SUCCESS)));
    EXPECT_CALL(*file, AsyncIO).WillOnce([&](auto ctx)
        {
            return EID(MFS_FILE_WRITE_FAILED);
        });
    EXPECT_CALL(*file, Close()).WillOnce(Return(EID(SUCCESS)));

    int expectValue = EID(MFS_FILE_WRITE_FAILED);
    EXPECT_EQ(VolumeMetaIntf::SaveVolumes(list, ARRAY_NAME, ARRAY_ID, move(file)), expectValue);
}

TEST_F(VolumeMetaIntfFixture, SaveVolumes_testIfAnErrorCodeWillBeReturnedWhenCloseRequestIsFailed)
{
    EXPECT_CALL(*file, DoesFileExist()).WillOnce(Return(false));
    EXPECT_CALL(*file, Create).WillOnce(Return(EID(SUCCESS)));
    EXPECT_CALL(*file, Open).WillOnce(Return(EID(SUCCESS)));
    EXPECT_CALL(*file, AsyncIO).WillOnce([&](auto ctx)
        {
            ctx->HandleIoComplete(ctx);
            return EID(SUCCESS);
        });
    EXPECT_CALL(*file, Close()).WillOnce(Return(EID(MFS_FILE_NOT_OPENED)));

    int expectValue = EID(VOL_UNABLE_TO_CLOSE_FILE);
    EXPECT_EQ(VolumeMetaIntf::SaveVolumes(list, ARRAY_NAME, ARRAY_ID, move(file)), expectValue);
}
} // namespace pos

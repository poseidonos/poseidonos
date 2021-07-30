#include "src/metafs/metafs.h"
#include "test/unit-tests/metafs/include/metafs_mock.h"
#include "test/unit-tests/metafs/mai/metafs_file_control_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_io_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_management_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_wbt_api_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"
#include <gtest/gtest.h>
#include <string>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::ReturnRef;

using namespace std;

namespace pos
{
class MetaFsFixture : public ::testing::Test
{
public:
    MetaFsFixture(void)
    {
    }

    virtual ~MetaFsFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        ptnSize.totalStripes = 100;
        ptnSize.blksPerStripe = 10;

        arrayInfo = new NiceMock<MockIArrayInfo>;
        mgmt = new NiceMock<MockMetaFsManagementApi>;
        ctrl = new NiceMock<MockMetaFsFileControlApi>;
        io = new NiceMock<MockMetaFsIoApi>;
        wbt = new NiceMock<MockMetaFsWBTApi>;
        mss = new NiceMock<MockMetaStorageSubsystem>(arrayId);

        for (int i = 0; i < MetaFsGeometryInfo::MAX_INFO_COUNT; ++i)
        {
            mediaInfoList[i].valid = false;
        }

        metaFs = new MetaFs(arrayInfo, false, mgmt, ctrl, io, wbt);
    }

    virtual void
    TearDown(void)
    {
        delete metaFs;
        delete arrayInfo;
    }

protected:
    MetaFs* metaFs;
    NiceMock<MockIArrayInfo>* arrayInfo;
    NiceMock<MockMetaFsManagementApi>* mgmt;
    NiceMock<MockMetaFsFileControlApi>* ctrl;
    NiceMock<MockMetaFsIoApi>* io;
    NiceMock<MockMetaFsWBTApi>* wbt;
    NiceMock<MockMetaStorageSubsystem>* mss;

    PartitionLogicalSize ptnSize;
    MetaFsStorageIoInfoList mediaInfoList;
    string fileName = "TestFile";
    uint64_t fileSize = 100;
    int arrayId = 0;
};

TEST_F(MetaFsFixture, InitMetaFs)
{
    EXPECT_CALL(*mgmt, InitializeSystem(_,_)).WillOnce(Return(POS_EVENT_ID::SUCCESS));
    EXPECT_CALL(*mgmt, GetMss).WillOnce(Return(mss));
    EXPECT_CALL(*io, SetMss);
    EXPECT_CALL(*ctrl, SetMss);
    EXPECT_CALL(*arrayInfo, GetSizeInfo).WillRepeatedly(Return(&ptnSize));
    EXPECT_CALL(*mgmt, GetAllStoragePartitionInfo)
        .WillRepeatedly(ReturnRef(mediaInfoList));
    EXPECT_CALL(*ctrl, CreateVolume).WillRepeatedly(Return(true));
    EXPECT_CALL(*mgmt, CreateMbr).WillRepeatedly(Return(true));
    EXPECT_CALL(*ctrl, OpenVolume).WillOnce(Return(true));
    EXPECT_CALL(*io, AddArray).WillOnce(Return(true));
    EXPECT_CALL(*mgmt, SetStatus);
    EXPECT_CALL(*io, SetStatus);
    EXPECT_CALL(*ctrl, SetStatus);
    EXPECT_CALL(*wbt, SetStatus);

    EXPECT_EQ(metaFs->Init(), 0);
}

TEST_F(MetaFsFixture, CheckFlush)
{
    metaFs->Flush();
}

TEST_F(MetaFsFixture, CheckDispose)
{
    EXPECT_CALL(*ctrl, CloseVolume(_)).WillOnce(Return(true));

    metaFs->Dispose();

    EXPECT_EQ(metaFs->GetMss(), nullptr);
}

TEST_F(MetaFsFixture, CheckShutdown_Without_Storage)
{
    EXPECT_CALL(*io, RemoveArray(_));

    metaFs->Shutdown();

    EXPECT_EQ(metaFs->GetMss(), nullptr);
}

TEST_F(MetaFsFixture, CheckShutdown_With_Storage)
{
    EXPECT_CALL(*mgmt, InitializeSystem(_,_)).WillOnce(Return(POS_EVENT_ID::SUCCESS));
    EXPECT_CALL(*mgmt, GetMss).WillOnce(Return(mss));
    EXPECT_CALL(*io, SetMss).WillRepeatedly(Return());
    EXPECT_CALL(*ctrl, SetMss).WillRepeatedly(Return());
    EXPECT_CALL(*arrayInfo, GetSizeInfo).WillRepeatedly(Return(&ptnSize));
    EXPECT_CALL(*mgmt, GetAllStoragePartitionInfo)
        .WillRepeatedly(ReturnRef(mediaInfoList));
    EXPECT_CALL(*ctrl, CreateVolume).WillRepeatedly(Return(true));
    EXPECT_CALL(*mgmt, CreateMbr).WillRepeatedly(Return(true));
    EXPECT_CALL(*ctrl, OpenVolume).WillOnce(Return(true));
    EXPECT_CALL(*io, AddArray).WillOnce(Return(true));
    EXPECT_CALL(*mgmt, SetStatus);
    EXPECT_CALL(*io, SetStatus);
    EXPECT_CALL(*ctrl, SetStatus);
    EXPECT_CALL(*wbt, SetStatus);

    EXPECT_EQ(metaFs->Init(), 0);

    EXPECT_CALL(*io, RemoveArray(_));
    EXPECT_CALL(*mss, Close);

    metaFs->Shutdown();

    EXPECT_EQ(metaFs->GetMss(), nullptr);
}

TEST_F(MetaFsFixture, CheckEpochSignature)
{
    EXPECT_CALL(*mgmt, GetEpochSignature).WillOnce(Return(123456));

    EXPECT_EQ(metaFs->GetEpochSignature(), 123456);
}

} // namespace pos

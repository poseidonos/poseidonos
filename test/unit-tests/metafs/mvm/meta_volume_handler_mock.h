#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/meta_volume_handler.h"

namespace pos
{
class MockMetaVolumeHandler : public MetaVolumeHandler
{
public:
    using MetaVolumeHandler::MetaVolumeHandler;
    MOCK_METHOD(void, InitHandler, (MetaVolumeContainer* volContainer));
    MOCK_METHOD(POS_EVENT_ID, HandleOpenFileReq,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleCloseFileReq,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleCreateFileReq,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleDeleteFileReq,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleGetDataChunkSizeReq,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleCheckFileAccessibleReq,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleGetFileSizeReq,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleGetTargetMediaTypeReq,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleGetFileBaseLpnReq,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleGetFreeFileRegionSizeReq,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleCheckFileExist,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleCreateArrayReq,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleDeleteArrayReq,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleGetMaxMetaLpnReq,
        (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleGetMetaFileInodeListReq,
        (MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleGetFileInodeReq,
        (MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(POS_EVENT_ID, HandleEstimateDataChunkSizeReq,
        (MetaFsFileControlRequest& reqMsg));
};

} // namespace pos

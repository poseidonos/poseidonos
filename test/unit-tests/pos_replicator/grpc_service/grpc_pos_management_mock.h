#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/pos_replicator/grpc_service/grpc_pos_management.h"

namespace pos
{
class MockGrpcPosManagement : public GrpcPosManagement
{
public:
    using GrpcPosManagement::GrpcPosManagement;
    MOCK_METHOD(::grpc::Status, UpdateVoluemMeta, (::grpc::ServerContext * context, const ::pos_rpc::UpdateVoluemMetaRequest* request, ::pos_rpc::PosResponse* response), (override));
    MOCK_METHOD(::grpc::Status, GetVolumeList, (::grpc::ServerContext * context, const ::pos_rpc::GetVolumeListRequest* request, ::pos_rpc::VolumeListResponse* response), (override));
};

} // namespace pos

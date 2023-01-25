#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/pos_replicator/grpc_service/grpc_replication_controller.h"

namespace pos
{
class MockGrpcReplicationController : public GrpcReplicationController
{
public:
    using GrpcReplicationController::GrpcReplicationController;
    MOCK_METHOD(::grpc::Status, StartVolumeSync, (::grpc::ServerContext * context, const pos_rpc::StartVolumeSyncRequest* request, pos_rpc::StartVolumeSyncResponse* response), (override));
};

} // namespace pos

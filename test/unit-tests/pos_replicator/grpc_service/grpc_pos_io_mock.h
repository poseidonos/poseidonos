#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/pos_replicator/grpc_service/grpc_pos_io.h"

namespace pos
{
class MockGrpcPosIo : public GrpcPosIo
{
public:
    using GrpcPosIo::GrpcPosIo;
    MOCK_METHOD(::grpc::Status, WriteBlocks, (::grpc::ServerContext * context, const pos_rpc::WriteBlocksRequest* request, pos_rpc::WriteBlocksResponse* response), (override));
    MOCK_METHOD(::grpc::Status, WriteBlocksSync, (::grpc::ServerContext * context, const pos_rpc::WriteBlocksSyncRequest* request, pos_rpc::WriteBlocksSyncResponse* response), (override));
    MOCK_METHOD(::grpc::Status, WriteHostBlocks, (::grpc::ServerContext * context, const pos_rpc::WriteHostBlocksRequest* request, pos_rpc::WriteHostBlocksResponse* response), (override));
    MOCK_METHOD(::grpc::Status, WriteHostBlocksSync, (::grpc::ServerContext * context, const pos_rpc::WriteHostBlocksSyncRequest* request, pos_rpc::WriteHostBlocksSyncResponse* response), (override));
    MOCK_METHOD(::grpc::Status, ReadBlocks, (::grpc::ServerContext * context, const pos_rpc::ReadBlocksRequest* request, pos_rpc::ReadBlocksResponse* response), (override));
    MOCK_METHOD(::grpc::Status, ReadBlocksSync, (::grpc::ServerContext * context, const pos_rpc::ReadBlocksSyncRequest* request, pos_rpc::ReadBlocksSyncResponse* response), (override));
    MOCK_METHOD(::grpc::Status, CompleteHostWrite, (::grpc::ServerContext * context, const pos_rpc::CompleteHostWriteRequest* request, pos_rpc::CompleteHostWriteResponse* response), (override));
};

} // namespace pos

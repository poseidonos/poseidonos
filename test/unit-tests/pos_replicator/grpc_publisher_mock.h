#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/pos_replicator/grpc_publisher.h"

namespace pos
{
class MockGrpcPublisher : public GrpcPublisher
{
public:
    using GrpcPublisher::GrpcPublisher;
};

} // namespace pos

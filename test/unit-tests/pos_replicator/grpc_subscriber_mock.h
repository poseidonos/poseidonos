#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/pos_replicator/grpc_subscriber.h"

namespace pos
{
class MockGrpcSubscriber : public GrpcSubscriber
{
public:
    using GrpcSubscriber::GrpcSubscriber;
};

} // namespace pos
